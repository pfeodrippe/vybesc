// PluginVybeSC.cpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#include "SC_PlugIn.hpp"
#include "VybeSC.hpp"
#include "VybeSC_dltest_shim.h"

#include <sstream>
#include <dlfcn.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>

#include "jshm.hpp"

static InterfaceTable *ft;

// cmake --build . --config Debug && cp VybeSC_scsynth.scx ~/dev/games/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/sonic-pi/prebuilt/macos/universal/supercollider/Resources/plugins/VybeSC_scsynth.scx

static VybeSlice *vybe_slice;
static VybeHooks vybe_hooks;
static void *vybe_function_handle;

static VybeAllocator vybe_allocator;

typedef void (*next)(Unit*, void*, int);
typedef next (*get_next_fn)();
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

struct DlTestConfig
{
    bool call_generatec = false;
    bool async_start_server = true;
    bool run_start_server = true;
    const char *generatec_target = nullptr;
};

static void reset_dltest_stats(VybeSC_DLTestStats *stats)
{
    if (stats == nullptr)
    {
        return;
    }

    stats->alpha_exit_code = -1;
    stats->generatec_exit_code = -1;
    stats->start_server_exit_code = -1;
    stats->my_next_loaded = 0;
    stats->my_next_fn = nullptr;
}

static int run_jank_command(jank_entrypoint_fn entry, const std::vector<std::string> &args)
{
    std::vector<const char *> argv;
    argv.reserve(args.size() + 2);

    argv.push_back("shared-host");
    for (const auto &arg : args)
    {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);

    int argc = static_cast<int>(argv.size() - 1);
    int exit_code = entry(argc, argv.data());

    std::cout << "jank_entrypoint invoked with";
    for (size_t i = 1; i < argv.size() - 1; ++i)
    {
        std::cout << ' ' << argv[i];
    }
    std::cout << ", exit_code=" << exit_code << "\n" << std::flush;

    return exit_code;
}

static void run_jank_command_async(jank_entrypoint_fn entry, std::vector<std::string> args)
{
    std::thread([entry, args = std::move(args)]() mutable {
        run_jank_command(entry, args);
    }).detach();
}

static bool VybeSC_run_dltest_internal(const char *path, const DlTestConfig &config, VybeSC_DLTestStats *stats)
{
    reset_dltest_stats(stats);

    if (path == nullptr)
    {
        std::cout << "VybeSC_run_dltest_internal ERROR: null path provided\n"
                  << std::flush;
        return false;
    }

    void *handle = dlopen(path, RTLD_NOW);
    vybe_function_handle = handle;

    std::cout << "OLHA_TEST - " << path << "\n"
              << std::flush;

    if (!handle)
    {
        std::cout << "dlopen ERROR --=-=-=-=\n"
                  << std::flush;
        return false;
    }

    jank_entrypoint_fn entry = (jank_entrypoint_fn)dlsym(handle, "jank_entrypoint");
    if (entry == NULL)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        dlclose(handle);
        vybe_function_handle = nullptr;
        return false;
    }

    int alpha_exit_code = run_jank_command(entry, std::vector<std::string>{"alpha"});
    if (stats != nullptr)
    {
        stats->alpha_exit_code = alpha_exit_code;
    }

    if (config.call_generatec)
    {
        std::vector<std::string> generatec_args = {"generatec"};
        if (config.generatec_target != nullptr)
        {
            generatec_args.emplace_back(config.generatec_target);
        }
        int generatec_exit_code = run_jank_command(entry, generatec_args);
        if (stats != nullptr)
        {
            stats->generatec_exit_code = generatec_exit_code;
        }
    }

    if (config.run_start_server)
    {
        if (config.async_start_server)
        {
            run_jank_command_async(entry, std::vector<std::string>{"start-server"});
            if (stats != nullptr)
            {
                stats->start_server_exit_code = 0;
            }
        }
        else
        {
            int start_server_exit = run_jank_command(entry, std::vector<std::string>{"start-server"});
            if (stats != nullptr)
            {
                stats->start_server_exit_code = start_server_exit;
            }
        }
    }

    get_next_fn get_next = (get_next_fn)dlsym(handle, "my_multiplier");
    if (get_next == NULL)
    {
        std::cout << "ERR: Could not find my_multiplier symbol --=-=-=-= "
                  << "\n"
                  << dlerror() << std::flush;
        if (stats != nullptr)
        {
            stats->my_next_loaded = 0;
        }
        return false;
    }

    my_next = get_next();
    if (stats != nullptr)
    {
        stats->my_next_loaded = (my_next == NULL) ? 0 : 1;
        stats->my_next_fn = (my_next_fn_t)my_next;
    }

    if (my_next == NULL)
    {
        std::cout << "ERR: my_multiplier() returned NULL --=-=-=-= \n" << std::flush;
        return false;
    }

    return true;
}

void VybeSC_dltest(World *inWorld, void *inUserData, struct sc_msg_iter *args, void *replyAddr)
{
    // Args are
    //   - lib location (string)
    const char *path = args->gets();

    DlTestConfig config;
    config.call_generatec = true;
    config.async_start_server = true;
    config.run_start_server = true;
    config.generatec_target = path;

    if (!VybeSC_run_dltest_internal(path, config, nullptr))
    {
        std::cout << "VybeSC_dltest failed for path: " << (path ? path : "<null>") << "\n"
                  << std::flush;
    }
}

extern "C" __attribute__((visibility("default"))) int VybeSC_dltest_shim(const char *dylib_path, int call_generatec, VybeSC_DLTestStats *out_stats)
{
    DlTestConfig config;
    config.call_generatec = (call_generatec != 0);
    config.async_start_server = true;
    config.run_start_server = false;
    config.generatec_target = dylib_path;

    bool ok = VybeSC_run_dltest_internal(dylib_path, config, out_stats);
    return ok ? 0 : 1;
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
