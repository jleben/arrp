
#include <arrp/vst3/processor.h>
#include <arrp/vst3/controller.h>
#include <public.sdk/source/main/pluginfactory.h>

namespace arrp {
namespace vst3 {

static
Steinberg::FUnknown* vst_create_processor(void*)
{
    auto * processor = create_processor();
    processor->setControllerClass(Steinberg::FUID(@ARRP_VST3_CONTROLLER_UID@));
    return (Vst::IAudioProcessor*) processor;
}

static
Steinberg::FUnknown* vst_create_controller(void*)
{
    return (Vst::IEditController*) new Controller();
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

DEF_CLASS2 (
        INLINE_UID_FROM_FUID(FUID(@ARRP_VST3_CONTROLLER_UID@)),
        PClassInfo::kManyInstances,
        kVstComponentControllerClass,
        "@ARRP_VST3_NAME@Controller",
        0,
        "",
        "@ARRP_VST3_VERSION@",
        kVstVersionString,
        arrp::vst3::vst_create_controller
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
