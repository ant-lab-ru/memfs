#pragma once

#include "memfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    memfs_file_t*   f;
    uint16_t        f_count;

    memfs_volume_t* v;
    uint8_t         v_count;

    bool            init_done;

    char            log_buf[MEMFS_LOG_STRING_MAX_LEN];

    uint8_t         data_buf[MEMFS_DATA_BUF_LEN];
} memfs_ctx_t;

typedef struct {
    uint32_t ptr0;
    uint32_t ptr1;
    uint32_t ptr2;
} memfs_log_header_t;

extern memfs_ctx_t memfs_ctx;

#define MEMFS_MIN(x,y) (((x) < (y)) ? (x) : (y))

#ifdef __cplusplus
}
#endif

