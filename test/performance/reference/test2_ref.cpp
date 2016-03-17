 
#include <cmath>
#include <algorithm>
using namespace std;

namespace test2_ref {

static const int K = 32;

struct alignas(16) state
{
    double in[K];
    int phase = 0;
};

inline void output( double * );

inline void initialize( state * s )
{
    s->in[0] = 0;
    for (int k = 1; k < K; ++k)
        s->in[k] = s->in[k-1] + 1;
    s->phase = 0;
}

inline void process( state * s )
{
    int ph = s->phase;

    double out = s->in[ph];
    for(int i = 1; i < K; ++i)
    {
        out += s->in[(ph+i)%K] * i;
    }
    output(&out);

    s->in[ph] = s->in[(K+ph-1)%K] + 1;
    ph = (ph + 1) % K;
    s->phase = ph;
}

} // namespace test2_ref

