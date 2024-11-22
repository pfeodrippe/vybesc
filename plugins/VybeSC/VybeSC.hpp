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

// Real-time allocation provided by SC.
struct VybeAllocator {
    void* (*alloc)(World* inWorld, size_t inSize);
    void (*free)(World* inWorld, void* inPtr);
};

// Runtime hooks for the dynamic lib.
struct VybeHooks {
    void (*ctor)(Unit*, VybeAllocator*);
    void (*dtor)(Unit*, VybeAllocator*);
    void (*next)(Unit*, int);
};

typedef void (*vybe_plugin_func)(Unit*, int);

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

        //vybe_plugin_func m_function_pointer;
    };

} // namespace VybeSC
