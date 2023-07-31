#pragma once

#include "memfs.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t _memfs_calculate_crc(memfs_file_t* f);

#ifdef __cplusplus
}
#endif

