#include "interface.h"
#include <pcl.h>

namespace arrp {
namespace puredata_io {

Abstract_IO * create_kernel();

static t_class * my_class;

struct my_object_type
{
    t_object base;
    t_float f;
    vector<char> kernel_stack;
    coroutine_t kernel_routine;
    bool has_routine;
    Abstract_IO * kernel;
    vector<t_sample*> inlet_buffers;
    vector<t_sample*> outlet_buffers;
    int buf_size;
};

static void * create_pd_object()
{
    printf("Create\n");

    my_object_type * object = reinterpret_cast<my_object_type*>(pd_new(my_class));
    // FIXME: Review stack size
    object->kernel_stack.resize(1024 * 1024);
    object->has_routine = false;
    object->kernel = create_kernel();

    printf("Num in %lu out %lu\n",
           object->kernel->inputs().size(),
           object->kernel->outputs().size());

    for (int i = 1; i < object->kernel->inputs().size(); ++i)
    {
        inlet_new(&object->base, &object->base.ob_pd, &s_signal, &s_signal);
    }

    for (int i = 0; i < object->kernel->outputs().size(); ++i)
    {
        outlet_new(&object->base, &s_signal);
    }

    return object;
}

static void destroy_pd_object(my_object_type * object)
{
    printf("Destroy\n");

    delete object->kernel;

    if (object->has_routine)
        co_delete(object->kernel_routine);
}

static
t_int* process_pd_signals(t_int* args)
{
    printf("Process\n");

    auto * object = reinterpret_cast<my_object_type*>(args[1]);

    // Reset up buffers

    for (int i = 0; i < object->kernel->inputs().size(); ++i)
    {
        auto & buf = object->kernel->inputs()[i];
        buf = Linear_Buffer<float>(object->inlet_buffers[i], object->buf_size);
    }

    for (int i = 0; i < object->kernel->outputs().size(); ++i)
    {
        auto & buf = object->kernel->outputs()[i];
        buf = Linear_Buffer<float>(object->outlet_buffers[i], object->buf_size);
    }

    // FIXME: Make sure the routine hasn't spontaneously ended already.
    co_call(object->kernel_routine);

    // Get data from buffers

    return args + 2;
}

static void kernel_routine_func(void * data)
{
    printf("Kernel routine\n");

    auto * object = static_cast<my_object_type*>(data);
    object->kernel->process(object->buf_size);
}

static void setup_pd_process_callback(my_object_type * object, t_signal **sp)
{
    printf("Setup DSP\n");

    if (object->has_routine)
        co_delete(object->kernel_routine);

    object->kernel_routine =
            co_create(kernel_routine_func, object,
                      object->kernel_stack.data(), object->kernel_stack.size());
    object->has_routine = true;

    object->buf_size = sp[0]->s_n;

    object->inlet_buffers.clear();
    for (int i = 0; i < object->kernel->inputs().size(); ++i, ++sp)
    {
        object->inlet_buffers.push_back((*sp)->s_vec);
    }

    object->outlet_buffers.clear();
    for (int i = 0; i < object->kernel->outputs().size(); ++i, ++sp)
    {
        object->outlet_buffers.push_back((*sp)->s_vec);
    }

    dsp_add(process_pd_signals, 1, object);
}

}
}

extern "C" {

// FIXME: setup function name must match object (library) name
void arrp_tilde_setup()
{
    printf("Library setup\n");

    using namespace arrp::puredata_io;

    // FIXME: object name
    my_class = class_new(gensym("arrp~"),
        (t_newmethod)create_pd_object,
        (t_method)destroy_pd_object,
        sizeof(my_object_type),
        CLASS_DEFAULT,
        (t_atomtype)0);

  class_addmethod(my_class,
                  (t_method)setup_pd_process_callback, gensym("dsp"), A_CANT, 0);

  CLASS_MAINSIGNALIN(my_class, my_object_type, f);
}

}
