#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <dlfcn.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

#include "plugins/VybeSC/VybeSC_dltest_shim.h"

#ifndef VYBESC_PLUGIN_PATH
#error "VYBESC_PLUGIN_PATH is not defined"
#endif

namespace
{
    constexpr const char *kDefaultPitocoPath = "/Users/pfeodrippe/dev/something/pitoco.dylib";

    const char *resolve_pitoco_path()
    {
        if (const char *env = std::getenv("VYBESC_PITOCO_PATH"))
        {
            return env;
        }
        return kDefaultPitocoPath;
    }

    void *load_plugin()
    {
        void *handle = dlopen(VYBESC_PLUGIN_PATH, RTLD_NOW);
        if (!handle)
        {
            throw std::runtime_error(std::string("Failed to dlopen VybeSC plugin: ") + dlerror());
        }
        return handle;
    }
}

int main()
{
    const char *pitoco_path = resolve_pitoco_path();

    if (access(pitoco_path, R_OK) != 0)
    {
        std::cerr << "Unable to access required dylib at " << pitoco_path << ": " << std::strerror(errno) << std::endl;
        return 1;
    }

    void *plugin_handle = nullptr;
    try
    {
        plugin_handle = load_plugin();
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    using ShimFn = int (*)(const char *, int, VybeSC_DLTestStats *);
    auto shim = reinterpret_cast<ShimFn>(dlsym(plugin_handle, "VybeSC_dltest_shim"));
    if (!shim)
    {
        std::cerr << "Could not find VybeSC_dltest_shim: " << dlerror() << std::endl;
        dlclose(plugin_handle);
        return 1;
    }

    VybeSC_DLTestStats stats{};
    int rc = shim(pitoco_path, 1, &stats);
    if (rc != 0)
    {
        std::cerr << "VybeSC_dltest_shim returned failure for path " << pitoco_path << std::endl;
        dlclose(plugin_handle);
        return 1;
    }

    if (!stats.my_next_loaded)
    {
        std::cerr << "Expected my_next to be loaded after dltest shim invocation" << std::endl;
        dlclose(plugin_handle);
        return 1;
    }

    if (stats.generatec_exit_code == -1)
    {
        std::cerr << "generatec command did not run" << std::endl;
        dlclose(plugin_handle);
        return 1;
    }

    dlclose(plugin_handle);
    return 0;
}
