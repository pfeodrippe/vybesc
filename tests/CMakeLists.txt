################################################################################
# CMakeLists.txt for VybeSC Tests
################################################################################

# Enable testing
enable_testing()

# Test: Dynamic Library Loading
add_executable(test_dlopen test_dlopen.cpp)
target_compile_features(test_dlopen PRIVATE cxx_std_17)
target_compile_options(test_dlopen PRIVATE -g3 -O0 -fno-omit-frame-pointer)

# Link with system libraries for backtrace
target_link_libraries(test_dlopen PRIVATE
    dl
)

# Add the test
add_test(
    NAME dlopen_test
    COMMAND test_dlopen
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Set test properties
set_tests_properties(dlopen_test
    PROPERTIES
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "All tests PASSED"
    FAIL_REGULAR_EXPRESSION "FAILED"
)
