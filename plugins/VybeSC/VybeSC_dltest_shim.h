#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VybeSC_DLTestStats {
    int alpha_exit_code;
    int generatec_exit_code;
    int start_server_exit_code;
    int my_next_loaded;
} VybeSC_DLTestStats;

int VybeSC_dltest_shim(const char *dylib_path, int call_generatec, VybeSC_DLTestStats *out_stats);

#ifdef __cplusplus
}
#endif
