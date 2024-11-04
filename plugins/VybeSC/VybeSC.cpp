// PluginVybeSC.cpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#include "SC_PlugIn.hpp"
#include "VybeSC.hpp"
#include <janet.h>
#include <sstream>
#include <dlfcn.h>
#include <iostream>

#include <jni.h>

static InterfaceTable* ft;

typedef int (*plugin_func)();

// cmake --build . --config Debug && cp VybeSC_scsynth.scx ~/dev/games/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/sonic-pi/prebuilt/macos/universal/supercollider/Resources/plugins/VybeSC_scsynth.scx

namespace VybeSC {

    // https://stackoverflow.com/questions/992836/how-to-access-the-java-method-in-a-c-application
    // https://github.com/openjdk/jdk/blob/master/test/hotspot/gtest/gtestMain.cpp#L94
    // https://www.iitk.ac.in/esc101/05Aug/tutorial/native1.1/implementing/method.html
    void vybe_sc_init_jvm() {
        JavaVM *vm;
        JNIEnv *env;
        JavaVMInitArgs vm_args;

        JavaVMOption* options = new JavaVMOption[1];
        options[0].optionString = "-Djava.class.path=.";
        vm_args.nOptions = 1;
        vm_args.options = options;

        vm_args.version = JNI_VERSION_21;
        vm_args.ignoreUnrecognized = false;

        // Construct a VM
        jint res = JNI_CreateJavaVM(&vm, (void **)&env, &vm_args);

        if (res == JNI_OK) {
            std::cout << "JVM OK \\o/ --=nn\n" << std::flush;
        }
        else {
            std::cout << "\n\nJVM ERRORRRR --=-= " << res << "\n\n\n" << std::flush;
            return;
        }

        // Construct a String
        jstring jstr = env->NewStringUTF("Hello World");

        // First get the class that contains the method you need to call
        jclass clazz = env->FindClass("java/lang/String");

        // Get the method that you want to call
        jmethodID to_lower = env->GetMethodID(clazz, "toLowerCase", "()Ljava/lang/String;");

        // Call the method on the object
        jobject result = env->CallObjectMethod(jstr, to_lower);

        // Get a C-style string
        const char* str = env->GetStringUTFChars((jstring) result, NULL);

        std::cout << str << std::flush;

        // Clean up
        env->ReleaseStringUTFChars(jstr, str);

        // Shutdown the VM.
        vm->DestroyJavaVM();
    }

    VybeSC::VybeSC() {
        mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
        next(1);

        vybe_sc_init_jvm();

        // janet_init();
        // jenv = janet_core_env(NULL);

        // vybe_sc_init_jvm();

        // Janet out;
        // int status = janet_dostring(jenv, "(eval-string (slurp \"/Users/pfeodrippe/dev/vybe/code.edn\"))", "main", &out);

        // Janet mainfun;

        //janet_resolve(jenv, janet_csymbol("my-fn"), &mainfun);
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

        //Janet jout;
        //janet_pcall(my_fn, 0, NULL, NULL, &fiber);
        //float myconst = janet_unwrap_number(jout);

        //Janet jout;
        //janet_dostring(jenv, "(eval-string (slurp \"/Users/pfeodrippe/dev/vybe/code.edn\"))", "main", &jout);
        //float myconst = janet_unwrap_number(jout);

        // Audio rate input
        const float* input = in(0);

        // Control rate parameter: gain.
        const float gain = 1.0f - in0(1);
        //const float gain = myconst - in0(1);

        // Output buffer
        float* outbuf = out(0);

        // simple gain function
        for (int i = 0; i < nSamples; ++i) {
            outbuf[i] = input[i] * gain;
        }
    }

} // namespace VybeSC

PluginLoad(VybeSCUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<VybeSC::VybeSC>(ft, "VybeSC", false);
}
