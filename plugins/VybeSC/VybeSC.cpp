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
        if (vybe_hooks.ctor == NULL) {
            std::cout << "WARN: vybe_hooks is not initialized yet!! VybeSC will be a noop until then\n" << std::flush;
            return;
        }
        (vybe_hooks.ctor)(this, &vybe_allocator);

        mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
        next(1);
    }

    VybeSC::~VybeSC() {
        if (vybe_hooks.dtor == NULL) {
            return;
        }
        (vybe_hooks.dtor)(this, &vybe_allocator);
    }

    void VybeSC::next(int nSamples) {
        (vybe_hooks.next)(this, nSamples);
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
        std::cout << "Could not find vybe_plugin_load\n" << std::flush;
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
