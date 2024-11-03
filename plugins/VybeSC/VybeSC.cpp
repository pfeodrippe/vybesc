// PluginVybeSC.cpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#include "SC_PlugIn.hpp"
#include "VybeSC.hpp"
#include <janet.h>

static InterfaceTable* ft;

namespace VybeSC {

VybeSC::VybeSC() {
    mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
    next(1);

    janet_init();
}

void VybeSC::next(int nSamples) {

    // Audio rate input
    const float* input = in(0);

    // Control rate parameter: gain.
    const float gain = 1.0f - in0(1);

    // Output buffer
    float* outbuf = out(0);

    // simple gain function
    for (int i = 0; i < nSamples; ++i) {
        outbuf[i] = input[i] * gain;
    }
}

} // namespace VybeSC

PluginLoad(VybeSCUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<VybeSC::VybeSC>(ft, "VybeSC", false);
}
