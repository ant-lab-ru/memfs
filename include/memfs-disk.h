#pragma once

#include "memfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int disk_erase(uint8_t volume, uint32_t addr, uint32_t size);
int disk_read(uint8_t volume, uint32_t addr, uint8_t* buffer, uint32_t length);
int disk_write(uint8_t volume, uint32_t addr, uint8_t* data, uint32_t size);
int disk_flush(uint8_t volume);

#ifdef __cplusplus
}
#endif
