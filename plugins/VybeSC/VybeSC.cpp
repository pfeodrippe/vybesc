// PluginVybeSC.cpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#include "SC_PlugIn.hpp"
#include "VybeSC.hpp"

#include <janet.h>
#include <sstream>
#include <dlfcn.h>
#include <iostream>

#include "jshm.hpp"

static InterfaceTable* ft;

// cmake --build . --config Debug && cp VybeSC_scsynth.scx ~/dev/games/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/sonic-pi/prebuilt/macos/universal/supercollider/Resources/plugins/VybeSC_scsynth.scx

static VybeSlice* vybe_slice;
static VybeHooks vybe_hooks;
static void* vybe_function_handle;

static VybeAllocator vybe_allocator;

namespace VybeSC {

    VybeSC::VybeSC() {
        (vybe_hooks.ctor)(this, &vybe_allocator);

        mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
        next(1);

        // Shared memory.
        // m_buffer = vybe_slice;
        // std::cout << (long)m_buffer << " - " << m_buffer->len << " - " << m_buffer->arr[0] << "\n" << std::flush;

        // Load library.
        // float ret;

        // std::cout << "Constructor arg " << in0(1) << "\n" << std::flush;

        // if (!vybe_function_handle) {
        //     std::cout << "dlopen ERROR --=-=-=-=\n" << std::flush;
        //     dlclose(vybe_function_handle);
        //     goto initialize;
        // }

        // if (m_function_pointer == NULL) {
        //     fprintf(stderr, "Could not find vybe_plugin_func: %s\n", dlerror());
        //     goto initialize;
        // }
        // if (dlclose(vybe_function_handle) != 0) {
        //     fprintf(stderr, "Could not close plugin: %s\n", dlerror());
        // }

        // Initialize.
    // initialize:
    //     mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
    //     next(1);
    }

    void VybeSC::next(int nSamples) {
        (vybe_hooks.next)(this, nSamples);

        // Audio rate input
        // const float* input = in(0);

        // // Control rate parameter: gain.
        // const float gain = in0(1);

        // // Output buffer
        // float* outbuf = out(0);

        // if (nSamples + m_buffer_current_pos - 1 >= m_buffer->len) {
        //     m_buffer_current_pos = 0;
        // }

        // simple gain function
        // for (int i = 0; i < nSamples; ++i) {
        //     outbuf[i] = input[i] * gain;

        //     // m_buffer->arr[m_buffer_current_pos] = outbuf[i];
        //     // m_buffer->timeline[m_buffer_current_pos] = m_buffer_global_pos;
        //     // m_buffer_current_pos++;
        //     // m_buffer_global_pos++;
        // }

        //(*m_function_pointer)(this, nSamples);
    }
} // namespace VybeSC

// void VybeSC_set_shared_memory_path(VybeSC::VybeSC *unit, sc_msg_iter *args) {
//     std::cout << "MEMORY_PATH: " << args->gets() << "\n" << std::flush;
// }

// Check DemoUGens.cpp in the supercollider repo.
void VybeSC_plugin_cmd(World* inWorld, void* inUserData, struct sc_msg_iter* args, void* replyAddr) {
    //Print("->cmdDemoFunc %d\n", args->geti());
    //Print("->cmdDemoFunc %d\n", args->geti());
    vybe_slice = (VybeSlice*)jshm::shared_memory::open(args->gets(), 1024 * 1024 * 128)->address();
}

void VybeSC_dlopen(World* inWorld, void* inUserData, struct sc_msg_iter* args, void* replyAddr) {
    // Args are
    //   - lib location (string)
    //   - load function name (string)

    vybe_function_handle = dlopen(args->gets(), RTLD_NOW);

    if (!vybe_function_handle) {
        std::cout << "dlopen ERROR --=-=-=-=\n" << std::flush;
        dlclose(vybe_function_handle);
        return;
    }

    // Load plugin.
    vybe_allocator = {
        .alloc = ft->fRTAlloc,
        .free  = ft->fRTFree
    };

    auto vybe_plugin_load = (VybeHooks (*)(VybeAllocator*))dlsym(vybe_function_handle, args->gets());
    if (vybe_plugin_load == NULL) {
        fprintf(stderr, "Could not find vybe_plugin_load: %s\n", dlerror());
        return;
    }
    vybe_hooks = (*vybe_plugin_load)(&vybe_allocator);

    // TODO Load dtor

    // Load next.
    // m_function_pointer = (vybe_plugin_func)dlsym(vybe_function_handle, args->gets());
    // if (m_function_pointer == NULL) {
    //     fprintf(stderr, "Could not find vybe_plugin_func: %s\n", dlerror());
    //     return;
    // }
}

PluginLoad(VybeSCUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<VybeSC::VybeSC>(ft, "VybeSC", false);

    // Plugin command, use with `/cmd`.
    DefinePlugInCmd("/vybe_cmd", VybeSC_plugin_cmd, NULL);
    DefinePlugInCmd("/vybe_dlopen", VybeSC_dlopen, NULL);

    // We define a unit command here, an instance method that can be called from the client
    // by using `/u_cmd`.
    //DefineUnitCmd("VybeSC", "/set_shared_memory_path", (UnitCmdFunc)&VybeSC_set_shared_memory_path);
}
