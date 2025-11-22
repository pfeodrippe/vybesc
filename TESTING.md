# VybeSC Testing Guide

## Quick Test Run

To test if your dynamic library works with VybeSC:

```bash
./scripts/run_tests.sh
```

This will compile and run the test suite against `/Users/pfeodrippe/dev/something/pitoco.dylib`.

## What the Test Does

The test suite (`tests/test_dlopen.cpp`) simulates the exact workflow that `VybeSC_dltest` performs:

1. **Load the library** - Uses `dlopen()` to load your dynamic library
2. **Find jank_entrypoint** - Resolves the `jank_entrypoint` function symbol
3. **Find my_multiplier** - Resolves the `my_multiplier` function symbol
4. **Execute jank_entrypoint** - Calls it with initialization arguments
5. **Execute my_multiplier** - Calls it with audio processing parameters

## Test Output Examples

### ✅ Success

```
================================
All tests PASSED!
```

Every test shows `SUCCESS:` and the returned values.

### ❌ Library Not Found

```
ERROR: Library not found at path
Expected: /Users/pfeodrippe/dev/something/pitoco.dylib
dlopen error: dyld: Library not loaded...
```

**Fix**: Ensure the library exists at the correct path

### ❌ Symbol Not Found

```
FAILED: dlsym could not find 'jank_entrypoint'
Error: Symbol not found in flat namespace...
```

**Fix**: Compile your library with `-fvisibility=default`

### ❌ Runtime Exception

```
FAILED: Exception caught: [error message]
```

**Fix**: Check the exception message and debug your library

## Customizing the Test Path

To test a different dynamic library, edit `tests/test_dlopen.cpp`:

```cpp
const char* TEST_DYLIB_PATH = "/path/to/your/library.dylib";
```

Then rebuild:

```bash
./scripts/run_tests.sh
```

## Comprehensive Testing with Enhanced Diagnostics

For better error reporting during testing, set these environment variables:

```bash
export ASAN_OPTIONS=verbosity=2:halt_on_error=1
export UBSAN_OPTIONS=print_stacktrace=1
./build/tests/test_dlopen
```

This enables Address Sanitizer and Undefined Behavior Sanitizer diagnostics.

## Integration with SuperCollider

Once tests pass, you can use VybeSC in SuperCollider:

```supercollider
// Load the plugin
VybeSC.dlopen("/Users/pfeodrippe/dev/something/pitoco.dylib", "jank_entrypoint")

// Or use the test function directly
VybeSC.dltest("/Users/pfeodrippe/dev/something/pitoco.dylib")
```

## Test Files

- `tests/test_dlopen.cpp` - Main test suite
- `tests/CMakeLists.txt` - CMake configuration for tests
- `scripts/run_tests.sh` - Convenient test runner script
- `tests/README.md` - Detailed testing documentation

## Debugging Failed Tests

If a test fails:

1. **Check the library path** - Does the file exist?
   ```bash
   ls -lh /Users/pfeodrippe/dev/something/pitoco.dylib
   ```

2. **Check exported symbols** - Are the functions exported?
   ```bash
   nm -D /Users/pfeodrippe/dev/something/pitoco.dylib | grep -E "jank_entrypoint|my_multiplier"
   ```

3. **Run with extra diagnostics** - Enable sanitizers
   ```bash
   export ASAN_OPTIONS=verbosity=2:halt_on_error=1
   ./build/tests/test_dlopen
   ```

4. **Use lldb for debugging**
   ```bash
   lldb ./build/tests/test_dlopen
   (lldb) run
   (lldb) bt  # Print backtrace if it crashes
   ```

5. **Check plugin error output** - Look for detailed messages in stdout/stderr
