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

typedef int (*plugin_func)();

// cmake --build . --config Debug && cp VybeSC_scsynth.scx ~/dev/games/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/sonic-pi/prebuilt/macos/universal/supercollider/Resources/plugins/VybeSC_scsynth.scx

static VybeSlice* vybe_slice;

namespace VybeSC {

    VybeSC::VybeSC() {
        // Read
        m_buffer = vybe_slice;
        std::cout << (long)m_buffer << " - " << m_buffer->len << " - " << m_buffer->arr[0] << "\n" << std::flush;

        // Load library.
        void* handle = dlopen("/Users/pfeodrippe/dev/vybe/libmy.dylib", RTLD_NOW);
        int ret;
        plugin_func f;

        std::cout << "Constructor arg " << in0(1) << "\n" << std::flush;

        if (!handle) {
            std::cout << "dlopen ERROR --=-=-=-=\n" << std::flush;
            dlclose(handle);
            goto initialize;
        }

        f = (plugin_func)dlsym(handle, "olha");
        if (f == NULL) {
            fprintf(stderr, "Could not find plugin_func: %s\n", dlerror());
            goto initialize;
        }
        printf("Calling plugin\n");
        ret = (*f)();
        printf("Plugin returned %d\n", ret);
        if (dlclose(handle) != 0) {
            fprintf(stderr, "Could not close plugin: %s\n", dlerror());
        }

        // Initialize.
    initialize:
        mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
        next(1);
    }

    void VybeSC::next(int nSamples) {
        // Audio rate input
        const float* input = in(0);

        // Control rate parameter: gain.
        const float gain = 1.0f - in0(1);

        // Output buffer
        float* outbuf = out(0);

        if (nSamples + m_buffer_current_pos - 1 >= m_buffer->len) {
            m_buffer_current_pos = 0;
        }

        // simple gain function
        for (int i = 0; i < nSamples; ++i) {
            outbuf[i] = input[i] * gain;

            m_buffer->arr[m_buffer_current_pos] = outbuf[i];
            m_buffer->timeline[m_buffer_current_pos] = m_buffer_global_pos;
            m_buffer_current_pos++;
            m_buffer_global_pos++;
        }
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

PluginLoad(VybeSCUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<VybeSC::VybeSC>(ft, "VybeSC", false);

    // Plugin command, use with `/cmd`.
    DefinePlugInCmd("/vybe_cmd", VybeSC_plugin_cmd, NULL);

    // We define a unit command here, an instance method that can be called from the client
    // by using `/u_cmd`.
    //DefineUnitCmd("VybeSC", "/set_shared_memory_path", (UnitCmdFunc)&VybeSC_set_shared_memory_path);
}
