#include "test2_opt.cpp"
#include "test2_raw.cpp"
#include "../reference/test2_ref.cpp"
#include "test_driver.hpp"

#include <chrono>
#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

using namespace std;
using namespace chrono;
using test_clock = chrono::high_resolution_clock;

using fp_type = float;

#define DEBUG_OUTPUT 0

#if DEBUG_OUTPUT

#else
#define n_init_iter 100
#define n_steady_iter 1000000
#endif

fp_type dummy;

//static fp_type dummy_array[n_iter];


namespace test2_opt {
inline void output(fp_type * data)
{
#if DEBUG_OUTPUT
    cout << *data << endl;
#else
    dummy = *data;
#endif
}
}

namespace test2_raw {
inline void output(fp_type * data)
{
#if DEBUG_OUTPUT
    cout << *data << endl;
#else
    dummy = *data;
#endif
}
}

namespace test2_ref {
inline void output(fp_type * data)
{
#if DEBUG_OUTPUT
    cout << *data << endl;
#else
    dummy = *data;
#endif
}
}

struct timer2
{
    double resolution()
    {
        struct timespec ts;
        clock_getres(CLOCK_PROCESS_CPUTIME_ID, &ts);
        return (double) ts.tv_sec * 1000 * 1000 + (double) ts.tv_nsec / 1000.0;
    }

    double start()
    {
        struct timespec ts;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
        return (double) ts.tv_sec * 1000 * 1000 + (double) ts.tv_nsec / 1000.0;
    }

    double stop()
    {
        return start();
    }
};

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static void
display_pthread_attr(pthread_attr_t *attr, const char *prefix)
{
    int s, i;
    size_t v;
    void *stkaddr;
    struct sched_param sp;

   s = pthread_attr_getdetachstate(attr, &i);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getdetachstate");
    printf("%sDetach state        = %s\n", prefix,
            (i == PTHREAD_CREATE_DETACHED) ? "PTHREAD_CREATE_DETACHED" :
            (i == PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" :
            "???");

   s = pthread_attr_getscope(attr, &i);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getscope");
    printf("%sScope               = %s\n", prefix,
            (i == PTHREAD_SCOPE_SYSTEM)  ? "PTHREAD_SCOPE_SYSTEM" :
            (i == PTHREAD_SCOPE_PROCESS) ? "PTHREAD_SCOPE_PROCESS" :
            "???");

   s = pthread_attr_getinheritsched(attr, &i);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getinheritsched");
    printf("%sInherit scheduler   = %s\n", prefix,
            (i == PTHREAD_INHERIT_SCHED)  ? "PTHREAD_INHERIT_SCHED" :
            (i == PTHREAD_EXPLICIT_SCHED) ? "PTHREAD_EXPLICIT_SCHED" :
            "???");

   s = pthread_attr_getschedpolicy(attr, &i);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getschedpolicy");
    printf("%sScheduling policy   = %s\n", prefix,
            (i == SCHED_OTHER) ? "SCHED_OTHER" :
            (i == SCHED_FIFO)  ? "SCHED_FIFO" :
            (i == SCHED_RR)    ? "SCHED_RR" :
            "???");

   s = pthread_attr_getschedparam(attr, &sp);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getschedparam");
    printf("%sScheduling priority = %d\n", prefix, sp.sched_priority);
#if 0
   s = pthread_attr_getguardsize(attr, &v);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getguardsize");
    printf("%sGuard size          = %d bytes\n", prefix, v);

   s = pthread_attr_getstack(attr, &stkaddr, &v);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getstack");
    printf("%sStack address       = %p\n", prefix, stkaddr);
    printf("%sStack size          = 0x%x bytes\n", prefix, v);
#endif
}


struct test2_ref_test
{
    test2_ref::state s;

    void initialize()
    {
        test2_ref::initialize(&s);
        for (int i = 0; i < n_init_iter; ++i)
        {
            test2_ref::process(&s);
        }
    }

    void run()
    {
        for (int i = 0; i < n_steady_iter; ++i)
        {
            test2_ref::process(&s);
        }
    }
};

struct test2_opt_test
{
    test2_opt::state s;

    void initialize()
    {
        test2_opt::initialize(&s);
        for (int i = 0; i < n_init_iter; ++i)
        {
            test2_opt::process(&s);
        }
    }

    void run()
    {
        for (int i = 0; i < n_steady_iter; ++i)
        {
            test2_opt::process(&s);
        }
    }
};

struct meta_test
{
    volatile float x = 1.0;
    volatile float y = 1.0;

    uint64_t num_iter;

    meta_test(uint64_t n): num_iter(n) {}

    void initialize() {}

    void run()
    {
        for (uint64_t i = 0; i < num_iter; ++i)
        {
            x = x * y;
        }
    }
};

int main()
{
#if 0
    if (true)
    {
        {
            int min = sched_get_priority_min(SCHED_FIFO);
            int max = sched_get_priority_max(SCHED_FIFO);
            cout << "Min priority for SCHED_FIFO = " << min << endl;
            cout << "Max priority for SCHED_FIFO = " << max << endl;
        }
        {
            int min = sched_get_priority_min(SCHED_RR);
            int max = sched_get_priority_max(SCHED_RR);
            cout << "Min priority for SCHED_RR = " << min << endl;
            cout << "Max priority for SCHED_RR = " << max << endl;
        }

        {
            cpu_set_t cpus;
            CPU_ZERO(&cpus);
            CPU_SET(1, &cpus);
            int result = sched_setaffinity(0, sizeof(cpu_set_t), &cpus);
            if (result == -1)
                cout << "Failed to set process CPU affinity: "
                     << strerror(errno) << endl;
        }

        struct sched_param param;
        param.sched_priority = 99;

        int policy = SCHED_FIFO;
#if 0
        {
            int result = sched_setscheduler(getpid(), policy, &param);
            if (result == -1)
                cout << "Failed to set process scheduling: "
                     << strerror(errno) << endl;
        }
#endif
#if 0
        {
            int result = pthread_setschedparam(pthread_self(), policy, &param);
            if (result != 0)
                cout << "Failed to set thread scheduling."
                     << strerror(errno) << endl;
        }
#endif

        cout << "Thread attributes:" << endl;
        {
            pthread_attr_t attr;
            pthread_getattr_np(pthread_self(), &attr);
            display_pthread_attr(&attr, "\t");
        }
    }
#endif

#if 0
    {
        //cout << "== Meta ==" << endl;
        meta_test test(1000 * 1000);
        test.run();
        //test_driver<meta_test> driver;
        //driver.go(test, 10, 5000);
    }
#endif
#if 1
    {
        //cout << "== Optimized ==" << endl;
        test2_opt_test opt;
        opt.initialize();
        opt.run();
        //test_driver<test2_opt_test> driver;
        //driver.go(opt, 10, 5000);
    }
#endif
#if 0
    {
        cout << "== Reference ==" << endl;
        test2_ref_test ref;
        test_driver<test2_ref_test> driver;
        driver.go(ref, 10, 5000);
    }
#endif


#if 0
    timer2 t2;
    cout << "Timer 2 resolution: " << t2.resolution() << endl;

    for (int r = 0; r < n_reps; ++r)
    {
        double ref_ms, raw_ms, opt_ms;

        uint64_t ref_start, ref_stop;
        uint64_t opt_start, opt_stop;

        double ref_start_s, ref_stop_s;
        double opt_start_s, opt_stop_s;

        cout << "Reference..." << endl;
        {
            test2_ref::state s;
            test2_ref::initialize(&s);

            //auto start = test_clock::now();
            //ref_start = timer::start();
#if 1
            unsigned start_low, start_high;
            unsigned stop_low, stop_high;
            int i;
            dummy = 198273;

            ref_start_s = t2.start();
            {
                asm volatile ("CPUID\n\t"
                              "RDTSC\n\t"
                              "mov %%edx, %0\n\t"
                              "mov %%eax, %1\n\t": "=r" (start_high), "=r" (start_low)::
                              "%rax", "%rbx", "%rcx", "%rdx");

            }
#endif
#if 1
            for (i = 0; i < n_iter; ++i)
            {
                //dummy = dummy * 0.98;
                dummy_array[i] = i;
            }
#endif
#if 0
            for (i = 0; i < n_iter; ++i)
            {
                dummy_array[i] = i;
                //test2_ref::process(&s);
            }
#endif
#if 1
            {
                asm volatile("RDTSCP\n\t"
                             "mov %%edx, %0\n\t"
                             "mov %%eax, %1\n\t"
                             "CPUID\n\t": "=r" (stop_high), "=r" (stop_low):: "%rax",
                             "%rbx", "%rcx", "%rdx");
            }
            ref_stop_s = t2.stop();

            ref_start = ((uint64_t)start_high << 32) | start_low;
            ref_stop = ((uint64_t)stop_high << 32) | stop_low;
#endif
            //ref_stop = timer::stop();

            //auto end = test_clock::now();
            //ref_ms = duration<double,milli>(end - start).count();
        }
#if 0
        cout << "Raw..." << endl;
        {
            test2_raw::state s;
            test2_raw::initialize(&s);

            auto start = test_clock::now();
            for (int i = 0; i < iter; ++i)
            {
                test2_raw::process(&s);
            }
            auto end = test_clock::now();
            raw_ms = duration<double,milli>(end - start).count();
        }
#endif

#if 0
        cout << "Opt..." << endl;
        {
            test2_opt::state s;
            test2_opt::initialize(&s);

            //auto start = test_clock::now();
            opt_start_s = t2.start();
            opt_start = t.start();
            for (uint64_t i = 0; i < n_iter; ++i)
            {
                test2_opt::process(&s);
            }
            opt_stop = t.stop();
            opt_stop_s = t2.stop();
            //auto end = test_clock::now();
            //opt_ms = duration<double,milli>(end - start).count();
        }
#endif

        uint64_t ref_cy = ref_stop - ref_start;
        //uint64_t opt_cy = opt_stop - opt_start;

        double ref_s = ref_stop_s - ref_start_s;
        //double opt_s = opt_stop_s - opt_start_s;

        //cout << "Reference = " << ref_ms << " ms" << endl;
        cout << "Reference = " << ref_s << " s" << endl;
        cout << "Reference = " << ref_cy << " cycles" << endl;
        //cout << "Raw = " << raw_ms << " ms" << endl;

        //cout << "Optimized = " << opt_ms << " ms" << endl;
        //cout << "Optimized = " << opt_s << " s" << endl;
        //cout << "Optimized = " << opt_cy << " cycles" << endl;

        //cout << "Raw/Optimized ms = " << raw_ms/opt_ms << " x" << endl;
        //cout << "Reference/Optimized ms = " << ref_ms/opt_ms << " x" << endl;
        //cout << "Reference/Optimized cycles = " << (double)ref_cy/opt_cy << " x" << endl;
    }
#endif
}
