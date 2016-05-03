#include <iostream>
#include <thread>
#include <chrono>
#include <papi.h>

#define measure_start(h,l) \
    asm volatile \
    ("CPUID\n\t" \
    "RDTSC\n\t" \
    "mov %%edx, %0\n\t" \
    "mov %%eax, %1\n\t": \
    "=r" (h), "=r" (l):: \
    "%rax", "%rbx", "%rcx", "%rdx")

#define measure_end(h,l) \
    asm volatile \
    ("RDTSCP\n\t" \
    "mov %%edx, %0\n\t" \
    "mov %%eax, %1\n\t" \
    "CPUID\n\t": \
    "=r" (h), "=r" (l):: \
    "%rax", "%rbx", "%rcx", "%rdx")

template <typename T>
class test_driver
{
public:
    uint64_t time(unsigned h, unsigned l)
    {
        return ((uint64_t)h << 32) | l;
    }

    double gettime_sec()
    {
        struct timespec ts;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
        return (double) ts.tv_sec * 1000 * 1000 + (double) ts.tv_nsec / 1000.0;
    }

    template <typename V>
    void compute_stats(V *d, int n, V & min, V & max, V & median)
    {
        using namespace std;
#if 0
        for (int i = 0; i < std::min(200, n); ++i)
        {
            cout << d[i] << endl;
        }
#endif
        sort(d, d+n);
        min = d[0];
        max = d[n-1];
        median = d[(int) ceil((n-1)/2.0)];
#if 0
        for (int i = 0; i < n; ++i)
        {
            auto v = d[i];
            if (min == 0 || v < min)
                min = v;
            if (v > max)
                max = v;
        }
#endif
    }

    template <typename V>
    void report_stats(const string & label, V *d, int n)
    {
        using namespace std;
        V min, max, median;
        compute_stats(d, n, min, max, median);
        cout << label << ":"
             << "  min = " << min
             << "  max = " << max
             << "  max/min = " << (double)max/min
             << "  median = " << median
             << endl;
    }

    void go(T & test, int n_rep, int n_ensemble)
    {
        using namespace std;
        using papi_cyc_t = long long;

        uint64_t * min_durs = new uint64_t[n_rep];
        uint64_t * durs = new uint64_t[n_ensemble];
        double * durs2 = new double[n_ensemble];
        double * durs3 = new double[n_ensemble];
        papi_cyc_t * durs4 = new papi_cyc_t[n_ensemble];
        papi_cyc_t * durs5 = new papi_cyc_t[n_ensemble];

        if (PAPI_library_init(PAPI_VER_CURRENT) < 0)
        {
            cerr << "ERROR: PAPI_library_init" << endl;
        }

        for (int r = 0; r < n_rep; ++r)
        {
            this_thread::sleep_for(chrono::milliseconds(500));

            for (int e = 0; e < n_ensemble; ++e)
            {
                unsigned start_cycle_h, start_cycle_l, end_cycle_h, end_cycle_l;

                double start_sec, end_sec;

                float real_time, proc_time, mflops;
                long long flpins;

                papi_cyc_t proc_cyc_start, proc_cyc_end;
                papi_cyc_t real_cyc_start, real_cyc_end;

                // Reset and warm up
                test.initialize();

                measure_start(start_cycle_h, start_cycle_l);
                measure_end(end_cycle_h, end_cycle_l);
                measure_start(start_cycle_h, start_cycle_l);
                measure_end(end_cycle_h, end_cycle_l);

                start_sec = gettime_sec();
#if 0
                if(PAPI_flops( &real_time, &proc_time, &flpins, &mflops) < PAPI_OK)
                {
                    cerr << "ERROR: PAPI_flops" << endl;
                    return;
                }
#endif
                //for (int i = 0; i < (r+1) * 100; ++i)
                measure_start(start_cycle_h, start_cycle_l);
                    proc_cyc_start = PAPI_get_virt_cyc();
                    real_cyc_start = PAPI_get_real_cyc();

                test.run();

                measure_end(end_cycle_h, end_cycle_l);
                proc_cyc_end = PAPI_get_virt_cyc();
                real_cyc_end = PAPI_get_real_cyc();
#if 0
                if(PAPI_flops( &real_time, &proc_time, &flpins, &mflops) < PAPI_OK)
                {
                    cerr << "ERROR: PAPI_flops" << endl;
                    return;
                }
#endif
                end_sec = gettime_sec();
#if 0
                if ( PAPI_stop_counters( nullptr, 0 ) != PAPI_OK )
                {
                    cerr << "ERROR: PAPI_stop_counters" << endl;
                    return;
                }
#endif

                uint64_t start = time(start_cycle_h, start_cycle_l);
                uint64_t end = time(end_cycle_h, end_cycle_l);
                uint64_t dur = end - start;

                durs[e] = dur;
                durs2[e] = end_sec - start_sec;
                //durs3[e] = proc_time;
                durs4[e] = proc_cyc_end - proc_cyc_start;
                durs5[e] = real_cyc_end - real_cyc_start;
            }

            uint64_t min_dur = 0;
            uint64_t max_dur = 0;

            double min_sec = 0;
            double max_sec = 0;

            double min_proc = 0;
            double max_proc = 0;

            papi_cyc_t min_proc_cyc, max_proc_cyc;
            papi_cyc_t min_real_cyc, max_real_cyc;

            //compute_stats(durs, n_ensemble, min_dur, max_dur);
            //compute_stats(durs2, n_ensemble, min_sec, max_sec);
            //compute_stats(durs4, n_ensemble, min_proc_cyc, max_proc_cyc);
            //compute_stats(durs5, n_ensemble, min_real_cyc, max_real_cyc);

            min_durs[r] = min_dur;

            cout << "Repetition " << r << ":  " << endl;

#if 0
            cout << "rdtsc: min = " << min_dur << "  max = " << max_dur
                 << "  max/min = " << (double) max_dur / min_dur
                 << endl;
#endif

#if 0
            cout << "papi high: min = " << min_proc << "  max = " << max_proc
                 << "  max/min = " << (double) max_proc / min_proc
                 << endl;
#endif

#if 0
            cout << "papi user: min = " << min_proc_cyc << "  max = " << max_proc_cyc
                 << "  max/min = " << (double) max_proc_cyc / min_proc_cyc
                 << endl;
#endif


            report_stats("rdtsc", durs, n_ensemble);
            report_stats("papi user", durs4, n_ensemble);
            report_stats("papi real", durs5, n_ensemble);

#if 0
            cout << "gettime: min = " << min_sec << "  max = " << max_sec
                 << "  max/min = " << (double) max_sec / min_sec
                 << endl;
#endif
        }

        {
            uint64_t sum_min_dur = 0;
            for (int r = 0; r < n_rep; ++r)
                sum_min_dur += min_durs[r];
            double avg_min_dur = (double) sum_min_dur / n_rep;
            cout << "avg min = " << avg_min_dur << endl;
        }

        delete[] durs;
        delete[] durs2;
        delete[] durs3;
        delete[] min_durs;
#if 0
        for (int r = 0; r < n_rep; ++r)
        {
            uint64_t min_dur = 0;
            uint64_t max_dur = 0;

            for (int e = 0; e < n_ensemble; ++e)
            {
                auto dur = durs[r][e];
                if (min_dur == 0 || dur < min_dur)
                    min_dur = dur;
                if (dur > max_dur)
                    max_dur = dur;
            }

            cout << "Repetition " << r << ": ";
            cout << "min=" << min_dur << "max=" << max_dur;
        }

        delete[] durs;
#endif
    }
};
