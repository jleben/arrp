
#include <arrp/vst3/processor.h>
#include <public.sdk/source/main/pluginfactory.h>

namespace arrp {
namespace vst3 {

static
Steinberg::FUnknown* vst_create_processor (void*)
{
    return (Vst::IAudioProcessor*) create_processor();
}

}
}

BEGIN_FACTORY_DEF (
        "@ARRP_VST3_COMPANY_NAME@",
        "@ARRP_VST3_COMPANY_WEB@",
        "@ARRP_VST3_COMPANY_EMAIL@"
)

DEF_CLASS2 (
        INLINE_UID_FROM_FUID(FUID(@ARRP_VST3_UID@)),
        PClassInfo::kManyInstances,
        kVstAudioEffectClass,
        "@ARRP_VST3_NAME@",
        Vst::kDistributable,
        "@ARRP_VST3_SUBCATEGORY@",
        "@ARRP_VST3_VERSION@",
        kVstVersionString,
        arrp::vst3::vst_create_processor
        )

END_FACTORY

bool InitModule ()
{
    return true;
}

bool DeinitModule ()
{
    return true;
}
