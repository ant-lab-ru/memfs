#pragma once

#include "memfs.h"

int _memfs_get_fid_by_name(const char* filename);
memfs_file_t* _memfs_get_file_by_fid(int fid);
void _memfs_load_default_file_info(memfs_file_t* f);
void _memfs_load_file_info_to_ctx(memfs_file_t* f);
bool _memfs_check_volumes(const memfs_volume_t* volumes, uint8_t volumes_count);
bool _memfs_check_files(const memfs_file_t* files, uint16_t files_count, uint8_t volumes_count);
