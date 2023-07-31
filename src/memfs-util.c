#include "memfs-disk.h"

#include "include/memfs-util.h"
#include "include/memfs-private.h"

#include <string.h>

int _memfs_get_fid_by_name(const char* filename)
{
    if (!filename) {
        return -1;
    }

    for (int i = 0; i < memfs_ctx.f_count; i++) {
        if (!strcmp(filename, memfs_ctx.f[i].name)) {
            return i;
        }
    }

    return -2;
}

memfs_file_t* _memfs_get_file_by_fid(int fid)
{
    if (fid < 0 || fid >= memfs_ctx.f_count) {
        return NULL;
    }

    return &memfs_ctx.f[fid];
}

void _memfs_load_default_file_info(memfs_file_t* f)
{
    f->crc = 0;
    f->file_valid = false;
    f->read_ptr = 0;
    f->size = 0;
    f->write_allowed = false;
    f->state = MEMFS_FILE_CLOSED;
}

void _memfs_load_file_info_to_ctx(memfs_file_t* f)
{
    _memfs_load_default_file_info(f);
    memfs_binary_file_header_t f_binhead = {0};
    memfs_file_header_t f_head = {0};
    int rc = 0;

    switch (f->attr)
    {
    case MEMFS_ATTR_BINARY:
        if (f->binary_file_header_offset == MEMFS_BINARY_FILE_WITHOUT_HEADER) {
            f->size = f->cap;
            break;
        }

        uint32_t head_addr = f->addr + f->binary_file_header_offset;
        rc = disk_read(f->volume_id, head_addr, (uint8_t*)&f_binhead, sizeof(f_binhead));
        if (rc != sizeof(f_binhead)) {
            f->size = f->cap;
            break;
        }
        if (f_binhead.file_size > f->cap) {
            f->size = f->cap;
            break;
        }
        f->size = f_binhead.file_size;
        f->crc = f_binhead.file_crc;
        f->file_valid = true;
        break;
    
    case MEMFS_ATTR_REGULAR:
    case MEMFS_ATTR_READ_ONLY:

        rc = disk_read(f->volume_id, f->addr, (uint8_t*)&f_head, sizeof(f_head));
        if (rc != sizeof(f_head)) {
            break;
        }
        if (!strcmp(f->name, f_head.file_name)) {
            break;
        }
        if (f_head.file_size > f->cap) {
            f->size = f->cap;
            break;
        }
        f->size = f_head.file_size;
        f->crc = f_head.file_crc;
        f->file_valid = true;
        break;

    case MEMFS_ATTR_ROOT_FILE:
        break;

    case MEMFS_ATTR_LOG_FILE:
        break;

    default:
        break;
    }

    return;
}

bool _memfs_check_volumes(const memfs_volume_t* volumes, uint8_t volumes_count)
{
    for (int i = 0; i < volumes_count; i++) {
        const memfs_volume_t* v = &volumes[i];

        if (v->type != MEMFS_VOLUME_TYPE_RAM) {
            return false;
        }
    }
    
    return true;
}

bool _memfs_check_files(const memfs_file_t* files, uint16_t files_count, uint8_t volumes_count)
{
    for (int i = 0; i < files_count; i++) {
        const memfs_file_t* f = &files[i];
        uint32_t bfh = f->binary_file_header_offset;

        if (f->volume_id >= volumes_count) {
            return false;
        }

        if (f->addr + f->cap < f->addr) {
            return false;
        }

        switch(f->attr)
        {
        case MEMFS_ATTR_BINARY:
            if (bfh == MEMFS_BINARY_FILE_WITHOUT_HEADER) {
                break;
            }

            if (bfh >= f->cap || bfh + sizeof(memfs_binary_file_header_t) > f->cap) {
                return false;
            }
            break;

        case MEMFS_ATTR_LOG_FILE:
            if(f->cap < MEMFS_LOG_STRING_MAX_LEN + sizeof(memfs_log_header_t)) {
                return false;
            }
            break;
        }
    }

    return true;
}

void _memfs_save_file_info_to_disk(memfs_file_t* f)
{
    memfs_file_header_t f_head = {0};
    int rc = 0;

    switch (f->attr)
    {
    case MEMFS_ATTR_REGULAR:
        f_head.file_crc = f->crc;
        f_head.file_size = f->size;
        strcpy(f_head.file_name, f->name);

        rc = disk_write(f->volume_id, f->addr, (uint8_t*)&f_head, sizeof(f_head));
        if (rc != sizeof(f_head)) {
            return;
        }
        break;
    
    default:
        break;
    }

    return;
}