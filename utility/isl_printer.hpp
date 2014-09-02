#ifndef STREAM_UTILITY_ISL_PRINTER_INCLUDED
#define STREAM_UTILITY_ISL_PRINTER_INCLUDED

#include <isl/printer.h>
#include <isl/set.h>
#include <isl/union_set.h>
#include <isl/map.h>
#include <isl/union_map.h>

#include <cstdio>

namespace stream {
namespace utility {
namespace isl {

class printer
{
public:
    printer(isl_ctx *ctx)
    {
        p = isl_printer_to_file(ctx, stdout);
    }

    ~printer()
    {
        isl_printer_free(p);
    }

    void print( isl_basic_set *set )
    {
        p = isl_printer_print_basic_set(p, set);
    }

    void print( isl_union_set *set )
    {
        p = isl_printer_print_union_set(p, set);
    }

    void print( isl_basic_map *map )
    {
        p = isl_printer_print_basic_map(p, map);
    }

    void print( isl_union_map *map )
    {
        p = isl_printer_print_union_map(p, map);
    }

private:
    isl_printer *p;
};

}
}
}

#endif // STREAM_UTILITY_ISL_PRINTER_INCLUDED
