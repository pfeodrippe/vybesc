# VybeSC Tests

This directory contains tests for the VybeSC SuperCollider plugin.

## test_dlopen.cpp

A comprehensive test suite for dynamic library loading and function resolution, designed to verify that the `VybeSC_dltest` function in `VybeSC.cpp` can properly load and interact with dynamic libraries.

### What It Tests

1. **Basic dlopen** - Verifies that `dlopen()` can load the dynamic library
2. **dlsym jank_entrypoint** - Checks that `jank_entrypoint` symbol can be resolved
3. **dlsym my_multiplier** - Checks that `my_multiplier` symbol can be resolved
4. **Execute jank_entrypoint** - Actually calls the `jank_entrypoint` function
5. **Execute my_multiplier** - Actually calls the `my_multiplier` function

### Configuration

By default, the test is configured to load:
```cpp
const char* TEST_DYLIB_PATH = "/Users/pfeodrippe/dev/something/pitoco.dylib";
```

Edit `test_dlopen.cpp` to change the path if needed.

### Running Tests

#### Quick Start
```bash
./scripts/run_tests.sh
```

#### Manual Build and Run
```bash
cd /path/to/vybesc
cmake --build build --config Debug --target test_dlopen
./build/tests/test_dlopen
```

#### Via CTest
```bash
cd build
ctest --output-on-failure --verbose
```

### Expected Output

All tests should show:
```
================================
All tests PASSED!
```

### Troubleshooting

**"dlopen ERROR"** - The library path is incorrect or the library doesn't exist
- Verify the path: `TEST_DYLIB_PATH`
- Check that the library file exists and is readable

**"dlsym could not find 'symbol'"** - The symbol is not exported from the dynamic library
- Ensure the library was compiled with `-fvisibility=default`
- Use `nm -D /path/to/library.dylib | grep symbol` to check available symbols

**"Exception caught"** - An error occurred during function execution
- Check the exception message for details
- If it's an unknown exception, the plugin's error handling will print more info
- Compile both the plugin and the library with sanitizer flags: `-fsanitize=address,undefined`

### Adding New Tests

To add a new test:

1. Create a new function following the pattern:
   ```cpp
   int test_something() {
       std::cout << "\n========== Test X: Description ==========\n";
       // your test code
       if (success) {
           std::cout << "SUCCESS: ...\n";
           return 0;
       } else {
           std::cout << "FAILED: ...\n";
           return 1;
       }
   }
   ```

2. Call it in `main()`:
   ```cpp
   failed_tests += test_something();
   ```

3. Rebuild and run tests

### Integration with VybeSC_dltest

This test suite simulates what the `VybeSC_dltest` function does in `VybeSC.cpp`:
- Opens a dynamic library via `dlopen(path, RTLD_NOW)`
- Resolves symbols via `dlsym(handle, "symbol_name")`
- Calls the loaded functions
- Handles exceptions and errors

The test helps verify that your dynamic library is compatible with the plugin before loading it in SuperCollider.
