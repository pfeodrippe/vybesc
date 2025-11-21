// PluginVybeSC.cpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#include "SC_PlugIn.hpp"
#include "VybeSC.hpp"

#include <sstream>
#include <dlfcn.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

#include "jshm.hpp"

static InterfaceTable *ft;

// cmake --build . --config Debug && cp VybeSC_scsynth.scx ~/dev/games/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/sonic-pi/prebuilt/macos/universal/supercollider/Resources/plugins/VybeSC_scsynth.scx

static VybeSlice *vybe_slice;
static VybeHooks vybe_hooks;
static void *vybe_function_handle;

static VybeAllocator vybe_allocator;

typedef void (*next)(Unit*, void*, int);
static next my_next = NULL;

// namespace VybeSC
// {

//     VybeSC::VybeSC()
//     {
//         if (vybe_hooks.ctor != NULL)
//         {
//             m_data = (vybe_hooks.ctor)(this, &vybe_allocator);
//         }

//         if (vybe_hooks.next == NULL)
//         {
//             std::cout << "WARN: vybe_hooks does not have a `next` hook function defined`!! VybeSC will be a noop until then\n"
//                       << std::flush;
//             return;
//         }

//         mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
//         next(1);
//     }

//     VybeSC::~VybeSC()
//     {
//         if (vybe_hooks.dtor != NULL)
//         {
//             (vybe_hooks.dtor)(this, m_data, &vybe_allocator);
//         }
//     }

//     void VybeSC::next(int nSamples)
//     {
//         (vybe_hooks.next)(this, m_data, nSamples);
//     }
// } // namespace VybeSC

namespace VybeSC
{

    VybeSC::VybeSC()
    {
        mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
        next(1);
    }

    VybeSC::~VybeSC()
    {
        // if (vybe_hooks.dtor != NULL)
        // {
        //     (vybe_hooks.dtor)(this, m_data, &vybe_allocator);
        // }
    }

    void VybeSC::next(int nSamples)
    {
        // Log every 5 seconds
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_last_log_time);
        if (elapsed.count() >= 1)
        {
            std::cout << "VybeSC::next called - buffer pos: " << m_buffer_current_pos << ", nSamples: " << nSamples << ", my_next: " << (my_next == NULL ? "NULL" : "ACTIVE") << "\n" << std::flush;
            m_last_log_time = now;
        }

        if (my_next == NULL)
        {
            return;
        }

        try
        {
            my_next(this, m_data, nSamples);
        }
        catch (const std::exception &e)
        {
            std::cout << "ERROR in my_next (std::exception): " << e.what() << "\n" << std::flush;
        }
        catch (const char *e)
        {
            std::cout << "ERROR in my_next (const char*): " << e << "\n" << std::flush;
        }
        catch (int e)
        {
            std::cout << "ERROR in my_next (int): " << e << "\n" << std::flush;
        }
        catch (...)
        {
            std::cout << "ERROR in my_next: Unknown exception caught (non-std::exception type)\n" << std::flush;
            std::cout << "  Possible causes:\n";
            std::cout << "  - Exception thrown from C code without proper wrapping\n";
            std::cout << "  - Segmentation fault or access violation\n";
            std::cout << "  - Stack overflow\n";
            std::cout << "  - Custom exception type not derived from std::exception\n" << std::flush;
        }
    }
} // namespace VybeSC

// void VybeSC_set_shared_memory_path(VybeSC::VybeSC *unit, sc_msg_iter *args) {
//     std::cout << "MEMORY_PATH: " << args->gets() << "\n" << std::flush;
// }

// Check DemoUGens.cpp in the supercollider repo.
void VybeSC_plugin_cmd(World *inWorld, void *inUserData, struct sc_msg_iter *args, void *replyAddr)
{
    // Print("->cmdDemoFunc %d\n", args->geti());
    // Print("->cmdDemoFunc %d\n", args->geti());
    vybe_slice = (VybeSlice *)jshm::shared_memory::open(args->gets(), 1024 * 1024 * 128)->address();
}

void VybeSC_dlopen(World *inWorld, void *inUserData, struct sc_msg_iter *args, void *replyAddr)
{
    // Args are
    //   - lib location (string)
    //   - load function name (string)

    vybe_function_handle = dlopen(args->gets(), RTLD_NOW);

    if (!vybe_function_handle)
    {
        std::cout << "dlopen ERROR --=-=-=-=\n"
                  << std::flush;
        dlclose(vybe_function_handle);
        return;
    }

    // Load plugin.
    vybe_allocator = {
        .alloc = ft->fRTAlloc,
        .free = ft->fRTFree};

    auto vybe_plugin_load = (VybeHooks (*)(VybeAllocator *))dlsym(vybe_function_handle, args->gets());
    if (vybe_plugin_load == NULL)
    {
        std::cout << "Could not find vybe_plugin_load\n"
                  << std::flush;
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

typedef int (*jank_entrypoint_fn)(int, const char **);

void VybeSC_dltest(World *inWorld, void *inUserData, struct sc_msg_iter *args, void *replyAddr)
{
    // Args are
    //   - lib location (string)
    //   // - load function name (string)
    auto path = args->gets();
    void *handle = dlopen(path, RTLD_NOW);
    vybe_function_handle = handle;

    std::cout << "OLHA_TEST - " << path << "\n"<< std::flush;

    if (!handle)
    {
        std::cout << "dlopen ERROR --=-=-=-=\n" << std::flush;
        dlclose(handle);
        return;
    }

    jank_entrypoint_fn entry = (jank_entrypoint_fn)dlsym(handle, "jank_entrypoint");
    if (entry == NULL)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        dlclose(handle);
        return;
    }

    std::vector<const char *> args1 = {
        "shared-host", // program name placeholder, dropped by the runtime
        "alpha",
        nullptr};

    std::vector<const char *> args2 = {
        "shared-host", // program name placeholder, dropped by the runtime
        "start-server",
        nullptr};

    int exit_code = entry(2, args1.data());
    std::cout << "AAA --=-=-=-= " << exit_code << "\n" << std::flush;

    std::thread([handle, entry, args2 = std::move(args2)]() mutable {
        std::cout << "BBB --=-=-=-=\n" << std::flush;
        int exit_code = entry(2, args2.data());
        std::cout << "entry returned " << exit_code << "\n" << std::flush;
    }).detach();

    std::cout << "CCC --=-=-=-=\n" << std::flush;

    my_next = (next)dlsym(handle, "my_multiplier");
    if (my_next == NULL)
    {
        std::cout << "ERR --=-=-=-= " << "\n" << dlerror() << std::flush;
    } else {
        std::cout << "DDD --=-=-=-= " << "\n" << std::flush;
    }

    // TODO Load dtor

    // Load next.
    // m_function_pointer = (vybe_plugin_func)dlsym(vybe_function_handle, args->gets());
    // if (m_function_pointer == NULL) {
    //     fprintf(stderr, "Could not find vybe_plugin_func: %s\n", dlerror());
    //     return;
    // }
}

PluginLoad(VybeSCUGens)
{
    // Plugin magic
    ft = inTable;
    registerUnit<VybeSC::VybeSC>(ft, "VybeSC", false);

    // Plugin command, use with `/cmd`.
    DefinePlugInCmd("/vybe_cmd", VybeSC_plugin_cmd, NULL);
    DefinePlugInCmd("/vybe_dlopen", VybeSC_dlopen, NULL);
    DefinePlugInCmd("/vybe_dltest", VybeSC_dltest, NULL);

    // We define a unit command here, an instance method that can be called from the client
    // by using `/u_cmd`.
    // DefineUnitCmd("VybeSC", "/set_shared_memory_path", (UnitCmdFunc)&VybeSC_set_shared_memory_path);
}
