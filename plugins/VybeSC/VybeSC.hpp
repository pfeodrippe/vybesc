// PluginVybeSC.hpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#pragma once

#include "SC_PlugIn.hpp"
#include <janet.h>

struct VybeSlice {
    long len;
    float* arr;
};

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
        VybeSlice m_buffer;
        long m_buffer_idx = 0;
    };

} // namespace VybeSC
