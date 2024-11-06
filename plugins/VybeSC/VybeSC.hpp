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

        // Other functions
        void init_jvm();

        // Member variables
        float* m_float_array;
    };

} // namespace VybeSC
