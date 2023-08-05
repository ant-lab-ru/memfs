#pragma once

#include "memfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int _memfs_get_fid_by_name(const char* filename);
memfs_file_t* _memfs_get_file_by_fid(int fid);
void _memfs_load_default_file_info(memfs_file_t* f);
void _memfs_load_file_info_to_ctx(memfs_file_t* f);
bool _memfs_check_volumes(const memfs_volume_t* volumes, uint8_t volumes_count);
bool _memfs_check_files(const memfs_file_t* files, uint16_t files_count, uint8_t volumes_count);
void _memfs_save_file_info_to_disk(memfs_file_t* f);

bool _memfs_is_read_mode(int mode);
bool _memfs_is_write_mode(int mode);
bool _memfs_is_append_mode(int mode);

#ifdef __cplusplus
}
#endif
