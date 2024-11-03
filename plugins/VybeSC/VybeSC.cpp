// PluginVybeSC.cpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#include "SC_PlugIn.hpp"
#include "VybeSC.hpp"
#include <janet.h>
#include <sstream>

static InterfaceTable* ft;

namespace VybeSC {

    VybeSC::VybeSC() {
        mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
        next(1);

        janet_init();
        jenv = janet_core_env(NULL);

        Janet out;
        int status = janet_dostring(jenv, "(defn my-fn [] -9900) (eval-string (slurp \"/Users/pfeodrippe/dev/vybe/code.edn\"))", "main", &out);

        Janet mainfun;
        janet_resolve(jenv, janet_csymbol("my-fn"), &mainfun);
        JanetFunction *my_fn = janet_unwrap_function(mainfun);
        JanetFiber *fiber = janet_fiber(my_fn, 64, 0, NULL);
        janet_gcroot(janet_wrap_fiber(fiber));
        fiber->env = jenv;
        Janet out2;
        janet_pcall(my_fn, 0, NULL, &out2, &fiber);

        std::ostringstream stringStream;
        // stringStream << "(spit \"/Users/pfeodrippe/dev/vybe/file.txt\" string(" << status << " \" \" " << janet_type(out) << "))";
        stringStream << "(spit \"/Users/pfeodrippe/dev/vybe/file.txt\" \"" << janet_unwrap_number(out) << " " << janet_unwrap_number(out2)  << "\")";
        std::string copyOfStr = stringStream.str();
        janet_dostring(jenv, copyOfStr.c_str(), "main", &out);
    }

    void VybeSC::next(int nSamples) {

        //Janet jout;
        //janet_dostring(jenv, "(eval-string (slurp \"/Users/pfeodrippe/dev/vybe/code.edn\"))", "main", &jout);
        //float myconst = janet_unwrap_number(jout);

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
