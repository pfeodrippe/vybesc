// PluginVybeSC.cpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#include "SC_PlugIn.hpp"
#include "VybeSC.hpp"

#include <janet.h>
#include <sstream>
#include <dlfcn.h>
#include <iostream>

static InterfaceTable* ft;

typedef int (*plugin_func)();

// cmake --build . --config Debug && cp VybeSC_scsynth.scx ~/dev/games/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/sonic-pi/prebuilt/macos/universal/supercollider/Resources/plugins/VybeSC_scsynth.scx

namespace VybeSC {

    VybeSC::VybeSC() {
        //m_buffer = (VybeSlice*)jshm::shared_memory::open("/tmp_vybe", 1024)->address();
        //std::cout << (long)m_buffer << " - " << m_buffer->len << "\n" << std::flush;

        mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
        next(1);

        // janet_init();
        // jenv = janet_core_env(NULL);

        // Janet out;
        // int status = janet_dostring(jenv, "(eval-string (slurp \"/Users/pfeodrippe/dev/vybe/code.edn\"))", "main", &out);

        // Janet mainfun;

        //janet_resolve(jenv, janet_csymbol("my-fn"), &mainfun);
        // JanetFunction *my_fn;
        //JanetCFunction my_fn = janet_unwrap_function(mainfun);

        // janet_resolve(jenv, janet_csymbol("olha"), &mainfun);
        // JanetCFunction my_fn = janet_unwrap_cfunction(mainfun);
        // my_fn(0, NULL);

        // fiber = janet_fiber(my_fn, 64, 0, NULL);
        // janet_gcroot(janet_wrap_fiber(fiber));
        // fiber->env = jenv;
        // Janet out2;
        // janet_pcall(my_fn, 0, NULL, &out2, &fiber);

        //Janet out2 = my_fn(0, NULL);

        // std::ostringstream stringStream;
        // // stringStream << "(spit \"/Users/pfeodrippe/dev/vybe/file.txt\" string(" << status << " \" \" " << janet_type(out) << "))";
        // stringStream << "(spit \"/Users/pfeodrippe/dev/vybe/file.txt\" \"" << status << " " << "\")";
        // std::string copyOfStr = stringStream.str();
        // janet_dostring(jenv, copyOfStr.c_str(), "main", &out);

        /////////////////////////// dlopen
        // void* handle = dlopen("/Users/pfeodrippe/dev/vybesc/libttt.dylib", RTLD_NOW);

        // if (!handle) {
        //     std::cout << "dlopen ERROR --=-=-=-=" << std::flush;
        //     dlclose(handle);
        // } else {
        //     std::cout << "dlopen GOOD --=-=-=-=" << std::flush;
        // }

        // plugin_func f = (plugin_func)dlsym(handle, "olha");
        // if (f == NULL) {
        //     fprintf(stderr, "Could not find plugin_func: %s\n", dlerror());
        // }
        // printf("Calling plugin\n");
        // int ret = (*f)();
        // printf("Plugin returned %d\n", ret);
        // if (dlclose(handle) != 0) {
        //     fprintf(stderr, "Could not close plugin: %s\n", dlerror());
        // }

        // std::ostringstream stringStream;
        // stringStream << "(spit \"/Users/pfeodrippe/dev/vybe/file.txt\" \"" << ret << " " << "\")";
        // std::string copyOfStr = stringStream.str();
        // janet_dostring(jenv, copyOfStr.c_str(), "main", NULL);
    }

    void VybeSC::next(int nSamples) {
        // Audio rate input
        const float* input = in(0);

        // Control rate parameter: gain.
        const float gain = 1.0f - in0(1);
        //const float gain = myconst - in0(1);

        // Output buffer
        float* outbuf = out(0);

        // if (nSamples + m_buffer_current_pos - 1 >= m_buffer.len) {
        //     m_buffer_current_pos = 0;
        // }

        // simple gain function
        for (int i = 0; i < nSamples; ++i) {
            outbuf[i] = input[i] * gain;

            // m_buffer.arr[m_buffer_current_pos] = outbuf[i];
            // m_buffer.timeline[m_buffer_current_pos] = m_buffer_global_pos;
            // m_buffer_current_pos++;
            // m_buffer_global_pos++;
        }
    }
} // namespace VybeSC

void VybeSC_set_shared_memory_path(VybeSC::VybeSC *unit, sc_msg_iter *args) {
    std::cout << "MEMORY_PATH: " << args->gets() << "\n" << std::flush;
}

PluginLoad(VybeSCUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<VybeSC::VybeSC>(ft, "VybeSC", false);

    // We defined a unit command here, an instance method that can be called from the client
    // by using /u_cmd
    DefineUnitCmd("VybeSC", "/set_shared_memory_path", (UnitCmdFunc)&VybeSC_set_shared_memory_path);
}
