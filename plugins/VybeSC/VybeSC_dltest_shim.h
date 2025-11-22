#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*my_next_fn_t)(void*, void*, int);

typedef struct VybeSC_DLTestStats {
    int alpha_exit_code;
    int generatec_exit_code;
    int start_server_exit_code;
    int my_next_loaded;
    my_next_fn_t my_next_fn;
} VybeSC_DLTestStats;

int VybeSC_dltest_shim(const char *dylib_path, int call_generatec, VybeSC_DLTestStats *out_stats);

#ifdef __cplusplus
}
#endif
