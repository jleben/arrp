 
#include <cmath>
#include <algorithm>
using namespace std;

namespace test2_ref {

static const int K = 32;

using fp_type = float;

struct alignas(16) state
{
    fp_type in[K];
    int phase = 0;
};

inline void output( fp_type * );

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

    fp_type out = s->in[ph];
    int i, p;
    for(i = 1, p=ph+1; p < K; ++i, ++p)
    {
        out += s->in[p] * i;
    }
    for(p=0; p < ph; ++i, ++p)
    {
        out += s->in[p] * i;
    }
    output(&out);

    s->in[ph] = s->in[(K+ph-1)%K] + 1;
    ph = (ph + 1) % K;
    s->phase = ph;
}

} // namespace test2_ref

