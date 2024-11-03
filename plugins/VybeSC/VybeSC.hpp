// PluginVybeSC.hpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#pragma once

#include "SC_PlugIn.hpp"
#include <janet.h>

namespace VybeSC {

    class VybeSC : public SCUnit {

    public:
        VybeSC();

        // Destructor
        // ~VybeSC();

    private:
        // Calc function
        void next(int nSamples);

        // Member variables
        JanetTable *jenv;
        JanetFunction *my_fn;
        //JanetCFunction my_fn;
        JanetFiber *fiber;
    };

} // namespace VybeSC
