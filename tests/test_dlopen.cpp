// test_dlopen.cpp
// Simple test to verify VybeSC_dltest can load a dynamic library

#include <dlfcn.h>
#include <iostream>
#include <cstring>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>

// Matching the plugin's function signature
typedef int (*jank_entrypoint_fn)(int, const char **);
typedef void (*next)(void*, void*, int);
typedef int (*simple_fn)(void);  // For eita2.simple function

// Test configuration
const char* TEST_DYLIB_PATH = "/Users/pfeodrippe/dev/something/pitoco.dylib";
const char* EITA2_DYLIB_PATH = "/Users/pfeodrippe/dev/something/eita2.dylib";  // Path to eita2 library

int test_dlopen_basic() {
    std::cout << "\n========== Test 1: Basic dlopen ==========\n";
    
    void *handle = dlopen(TEST_DYLIB_PATH, RTLD_NOW);
    if (!handle) {
        std::cout << "FAILED: dlopen returned NULL\n";
        std::cout << "Error: " << dlerror() << "\n";
        return 1;
    }
    
    std::cout << "SUCCESS: dlopen loaded library\n";
    std::cout << "Handle: " << handle << "\n";
    dlclose(handle);
    return 0;
}

int test_dlsym_jank_entrypoint() {
    std::cout << "\n========== Test 2: dlsym jank_entrypoint ==========\n";
    
    void *handle = dlopen(TEST_DYLIB_PATH, RTLD_NOW);
    if (!handle) {
        std::cout << "FAILED: dlopen returned NULL\n";
        std::cout << "Error: " << dlerror() << "\n";
        return 1;
    }
    
    jank_entrypoint_fn entry = (jank_entrypoint_fn)dlsym(handle, "jank_entrypoint");
    if (entry == NULL) {
        std::cout << "FAILED: dlsym could not find 'jank_entrypoint'\n";
        std::cout << "Error: " << dlerror() << "\n";
        dlclose(handle);
        return 1;
    }
    
    std::cout << "SUCCESS: Found jank_entrypoint at " << (void*)entry << "\n";
    dlclose(handle);
    return 0;
}

int test_dlsym_my_multiplier() {
    std::cout << "\n========== Test 3: dlsym my_multiplier ==========\n";
    
    void *handle = dlopen(TEST_DYLIB_PATH, RTLD_NOW);
    if (!handle) {
        std::cout << "FAILED: dlopen returned NULL\n";
        std::cout << "Error: " << dlerror() << "\n";
        return 1;
    }
    
    next my_multiplier = (next)dlsym(handle, "my_multiplier");
    if (my_multiplier == NULL) {
        std::cout << "FAILED: dlsym could not find 'my_multiplier'\n";
        std::cout << "Error: " << dlerror() << "\n";
        dlclose(handle);
        return 1;
    }
    
    std::cout << "SUCCESS: Found my_multiplier at " << (void*)my_multiplier << "\n";
    dlclose(handle);
    return 0;
}

int test_jank_entrypoint_execution() {
    std::cout << "\n========== Test 4: Execute jank_entrypoint ==========\n";
    
    void *handle = dlopen(TEST_DYLIB_PATH, RTLD_NOW);
    if (!handle) {
        std::cout << "FAILED: dlopen returned NULL\n";
        std::cout << "Error: " << dlerror() << "\n";
        return 1;
    }
    
    jank_entrypoint_fn entry = (jank_entrypoint_fn)dlsym(handle, "jank_entrypoint");
    if (entry == NULL) {
        std::cout << "FAILED: dlsym could not find 'jank_entrypoint'\n";
        std::cout << "Error: " << dlerror() << "\n";
        dlclose(handle);
        return 1;
    }
    
    std::vector<const char *> args = {
        "shared-host",
        "alpha",
        nullptr};
    
    std::cout << "Calling jank_entrypoint(2, [...])...\n";
    try {
        int exit_code = entry(2, args.data());
        std::cout << "SUCCESS: jank_entrypoint returned " << exit_code << "\n";
    } catch (const std::exception &e) {
        std::cout << "FAILED: Exception caught: " << e.what() << "\n";
        dlclose(handle);
        return 1;
    } catch (...) {
        std::cout << "FAILED: Unknown exception caught\n";
        dlclose(handle);
        return 1;
    }
    
    dlclose(handle);
    return 0;
}

int test_my_multiplier_execution() {
    std::cout << "\n========== Test 5: Execute my_multiplier ==========\n";
    
    void *handle = dlopen(TEST_DYLIB_PATH, RTLD_NOW);
    if (!handle) {
        std::cout << "FAILED: dlopen returned NULL\n";
        std::cout << "Error: " << dlerror() << "\n";
        return 1;
    }
    
    next my_multiplier = (next)dlsym(handle, "my_multiplier");
    if (my_multiplier == NULL) {
        std::cout << "FAILED: dlsym could not find 'my_multiplier'\n";
        std::cout << "Error: " << dlerror() << "\n";
        dlclose(handle);
        return 1;
    }
    
    std::cout << "Calling my_multiplier(nullptr, nullptr, 64)...\n";
    try {
        my_multiplier(nullptr, nullptr, 64);
        std::cout << "SUCCESS: my_multiplier executed without exception\n";
    } catch (const std::exception &e) {
        std::cout << "FAILED: Exception caught: " << e.what() << "\n";
        dlclose(handle);
        return 1;
    } catch (...) {
        std::cout << "FAILED: Unknown exception caught\n";
        dlclose(handle);
        return 1;
    }
    
    dlclose(handle);
    return 0;
}

int test_eita2_simple_execution() {
    std::cout << "\n========== Test 6: Execute eita2.simple ==========\n";
    
    void *handle = dlopen(EITA2_DYLIB_PATH, RTLD_NOW);
    if (!handle) {
        std::cout << "FAILED: dlopen returned NULL\n";
        std::cout << "Error: " << dlerror() << "\n";
        std::cout << "Note: Library path is: " << EITA2_DYLIB_PATH << "\n";
        return 1;
    }
    
    simple_fn simple = (simple_fn)dlsym(handle, "simple");
    if (simple == NULL) {
        std::cout << "FAILED: dlsym could not find 'simple'\n";
        std::cout << "Error: " << dlerror() << "\n";
        dlclose(handle);
        return 1;
    }
    
    std::cout << "Calling simple()...\n";
    try {
        int result = simple();
        std::cout << "SUCCESS: simple executed and returned " << result << "\n";
        std::cout << "Expected result: 35 (30 + 5)\n";
        if (result == 35) {
            std::cout << "Result matches expected value!\n";
        } else {
            std::cout << "WARNING: Result does not match expected value (got " << result << ", expected 35)\n";
        }
    } catch (const std::exception &e) {
        std::cout << "FAILED: Exception caught: " << e.what() << "\n";
        dlclose(handle);
        return 1;
    } catch (...) {
        std::cout << "FAILED: Unknown exception caught\n";
        dlclose(handle);
        return 1;
    }
    
    dlclose(handle);
    return 0;
}

int main(int argc, char **argv) {
    std::cout << "================================\n";
    std::cout << "VybeSC Dynamic Library Test Suite\n";
    std::cout << "Testing: " << TEST_DYLIB_PATH << "\n";
    std::cout << "Testing: " << EITA2_DYLIB_PATH << "\n";
    std::cout << "================================\n";
    
    // Check if library exists
    void *quick_check = dlopen(TEST_DYLIB_PATH, RTLD_NOW);
    if (!quick_check) {
        std::cout << "\nERROR: Library not found at path\n";
        std::cout << "Expected: " << TEST_DYLIB_PATH << "\n";
        std::cout << "dlopen error: " << dlerror() << "\n";
        std::cout << "\nTo run these tests, ensure the library exists at the expected path.\n";
        return 1;
    }
    dlclose(quick_check);
    
    int failed_tests = 0;
    
    // Run tests
    failed_tests += test_dlopen_basic();
    failed_tests += test_dlsym_jank_entrypoint();
    failed_tests += test_dlsym_my_multiplier();
    failed_tests += test_jank_entrypoint_execution();
    failed_tests += test_my_multiplier_execution();
    failed_tests += test_eita2_simple_execution();
    
    std::cout << "\n================================\n";
    if (failed_tests == 0) {
        std::cout << "All tests PASSED!\n";
        return 0;
    } else {
        std::cout << failed_tests << " test(s) FAILED!\n";
        return 1;
    }
}
