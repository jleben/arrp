#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace arrp {
namespace vst3 {

//-----------------------------------------------------------------------------
class Controller : public Vst::EditController
{
public:
    //---from IPluginBase--------
    //Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE

    //---from EditController-----
    //Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
};

}
}
