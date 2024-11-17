// PluginVybeSC.hpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#pragma once

#include "SC_PlugIn.hpp"
#include <janet.h>

struct VybeSlice {
    long len;
    float arr[4400];
    long timeline[4400];
};

typedef float (*plugin_func)();

namespace VybeSC {

    class VybeSC : public SCUnit {

    public:
        VybeSC();

        // Destructor
        // ~VybeSC();

        // void set_shared_memory_path(sc_msg_iter *args);

    private:
        // Calc function
        void next(int nSamples);

        // Other functions

        // Member variables
        VybeSlice* m_buffer;
        long m_buffer_current_pos = 0;
        long m_buffer_global_pos = 0;

        plugin_func m_function_pointer;
    };

} // namespace VybeSC
