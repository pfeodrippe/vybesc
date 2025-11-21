// PluginVybeSC.hpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#pragma once

#include "SC_PlugIn.hpp"

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
    // ctor returns the dyn lib data.
    void* (*ctor)(Unit*, VybeAllocator*);
    void (*dtor)(Unit*, void*, VybeAllocator*);
    void (*next)(Unit*, void*, int);
};

namespace VybeSC {

    class VybeSC : public SCUnit {

    public:
        VybeSC();

        // Destructor
        ~VybeSC();

        // void set_shared_memory_path(sc_msg_iter *args);

    private:
        // Calc function
        void next(int nSamples);

        // Other functions

        // Member variables
        VybeSlice* m_buffer;
        long m_buffer_current_pos = 0;
        long m_buffer_global_pos = 0;

        // Pointer for state from the dyn lib.
        void* m_data;

        // Timing for periodic logging
        std::chrono::steady_clock::time_point m_last_log_time = std::chrono::steady_clock::now();
    };

} // namespace VybeSC
