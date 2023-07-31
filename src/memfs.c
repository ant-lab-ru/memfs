#include "memfs.h"
#include "memfs-disk.h"
#include "include/memfs-private.h"

#include <string.h>

memfs_ctx_t memfs_ctx = {0};

int memfs_init(const memfs_file_t* files, const uint16_t files_count, const memfs_volume_t* volumes, const uint8_t volumes_count)
{
    memset(&memfs_ctx, 0, sizeof(memfs_ctx_t));

    if (!files || !volumes || !files_count || !volumes_count) {
        return -1;
    }

    /*
    *   Проверка на критические ошибки массива структур volumes,
    *   с которыми работа файловой системы невозможна
    */
    if (!_memfs_check_volumes(volumes, volumes_count)) {
        return -2;
    }

    /*
    *   Проверка на критические ошибки массива структур files,
    *   с которыми работа файловой системы невозможна
    */
    if (!_memfs_check_files(files, files_count, volumes_count)) {
        return -3;
    }

    memfs_ctx.v = volumes;
    memfs_ctx.v_count = volumes_count;
    memfs_ctx.f = files;
    memfs_ctx.f_count = files_count;

    /*
    *   Вычитываем информацию о файлах из энергонезависимой памяти
    */
    for (int i = 0; i < files_count; i++) {
        _memfs_load_file_info_to_ctx(&files[i]);
    }

    memfs_ctx.init_done = true;

    return 0;
}

int memfs_open(const char* filename, memfs_mode_t mode)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    int fid = _memfs_get_fid_by_name(filename);
    memfs_file_t* f = _memfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }
    
    if (f->state != MEMFS_FILE_CLOSED) {
        return -3;
    }

    if (mode == MEMFS_MODE_R) {
        f->read_ptr = 0;
        f->state = MEMFS_FILE_STATE_RO;
        return fid;
    }

    if (f->attr == MEMFS_ATTR_READ_ONLY || 
        f->attr == MEMFS_ATTR_ROOT_FILE ||
        f->attr == MEMFS_ATTR_LOG_FILE  ||
        !f->write_allowed) {
        return -1;
    }

    if (mode == MEMFS_MODE_W) {
        f->size = 0;
        f->state = MEMFS_FILE_STATE_WO;
        return fid;
    }
    
    if (mode == MEMFS_MODE_A) {
        if (f->attr == MEMFS_ATTR_BINARY) {
            return -1;
        }
        f->state = MEMFS_FILE_STATE_WO;
        return fid;
    }

    return -1;
}

int memfs_close(int fid)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    memfs_file_t* f = _emfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }

    if (f->state == MEMFS_FILE_CLOSED) {
        return -3;
    }

    f->state = MEMFS_FILE_CLOSED;

    if (f->state == MEMFS_FILE_STATE_RO) {
        return 0;
    }

    int rc = 0;

    if (f->attr == MEMFS_ATTR_REGULAR) {
        f->crc = _memfs_calculate_crc(f);
        _memfs_save_file_to_disk(f);
    }

    return rc;
}

int memfs_read(int fid, uint8_t* buffer, uint32_t length)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }

    memfs_file_t* f = _emfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }

    if (f->state != MEMFS_FILE_STATE_RO) {
        return -1;
    }

    if (f->read_ptr >= f->size || f->size > f->cap) {
        return -2;
    }

    uint32_t read_addr = f->addr + f->read_ptr;
    if (read_addr > f->addr + f->size) {
        return -2;
    }

    uint32_t read_len = MEMFS_MIN(length, f->size - f->read_ptr);

    int rc = disk_read(f->volume_id, read_addr, buffer, read_len);
    if (rc != read_len) {
        return -3;
    }

    f->read_ptr += read_len;

    return read_len;
}

int memfs_write(int fid, uint8_t* data, uint32_t size)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    memfs_file_t* f = _emfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }

    if (f->state != MEMFS_FILE_STATE_WO) {
        return -1;
    }

    if (f->size > f->cap) {
        return -3;
    }

    uint32_t write_addr = f->addr + f->size;
    if (write_addr > f->addr + f->cap) {
        return -2;
    }
    if (write_addr + size > f->addr + f->cap || size > f->cap) {
        return -2;
    }

    int rc = disk_write(f->volume_id, write_addr, data, size);
    if (rc != size) {
        return -3;
    }
    f->size += size;

    return size;
}

int memfs_seek(int fid, uint32_t offset)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    memfs_file_t* f = _emfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }

    if (f->state != MEMFS_FILE_STATE_RO) {
        return -1;
    }

    if (offset > f->cap) {
        return -2;
    }

    f->read_ptr = offset;

    return 0;
}

int memfs_unlock_file_write(const char* filename)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    int fid = _memfs_get_fid_by_name(filename);
    memfs_file_t* f = _memfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }
    
    if (f->state != MEMFS_FILE_CLOSED) {
        return -4;
    }

    if (f->attr != MEMFS_ATTR_REGULAR && f->attr != MEMFS_ATTR_BINARY) {
        return -3;
    }

    f->write_allowed = true;

    return 0;
}

int memfs_lock_file_write(const char* filename)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    int fid = _memfs_get_fid_by_name(filename);
    memfs_file_t* f = _memfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }

    if (f->state != MEMFS_FILE_CLOSED) {
        return -4;
    }

    f->write_allowed = false;

    return 0;
}

int memfs_stat(const char* filename, memfs_stat_t* stat)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    int fid = _memfs_get_fid_by_name(filename);
    memfs_file_t* f = _memfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }

    if (!stat) {
        return -1;
    }
    
    if (f->state != MEMFS_FILE_CLOSED) {
        return -4;
    }

    uint32_t calculated_crc = _memfs_calculate_crc(f);

    strcpy(stat->file_name, f->name);
    stat->cap = f->cap;
    stat->size = f->size;
    stat->addr = f->addr;
    stat->expected_crc = f->crc;
    stat->volume_id = f->volume_id;
    stat->calculated_crc = calculated_crc;
    stat->file_valid = f->file_valid && (calculated_crc == f->crc) && f->size;

    return 0;
}

int memfs_log_write(const char* filename, const char* format, ...)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    int fid = _memfs_get_fid_by_name(filename);
    memfs_file_t* f = _memfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }

    if (f->attr != MEMFS_ATTR_LOG_FILE) {
        return -2;
    }

    if (!f->file_valid) {
        return -3;
    }

    va_list args;
    va_start(args, format);
    vsprintf(memfs_ctx.log_buf, format, args);
    va_end(args);

    uint32_t len = strlen(memfs_ctx.log_buf);
    if (len > MEMFS_LOG_STRING_MAX_LEN) {
        return -3;
    }

    memfs_log_header_t log_head = {0};
    int rc = 0;
    rc = disk_read(f->volume_id, f->addr, &log_head, sizeof(log_head));
    if (rc != sizeof(log_head)) {
        return -4;
    }

    uint32_t ptr = 0;
    if (log_head.ptr1 == log_head.ptr2) {
        ptr = log_head.ptr1;
    }
    else {
        ptr = log_head.ptr0;
    }
    
    uint32_t data_start_addr = f->addr + sizeof(log_head);
    uint32_t data_end_addr = f->addr + f->cap;

    uint32_t write_addr = data_start_addr + ptr;

    if (write_addr >= data_end_addr || write_addr < data_start_addr) {
        ptr = 0;
        write_addr = data_start_addr;
    }

    if (write_addr + len > data_end_addr) {
        rc = disk_erase(f->volume_id, write_addr, data_end_addr - write_addr);
        if (rc < 0) {
            return -6;
        }
        ptr = 0;
        write_addr = data_start_addr;
    }

    rc = disk_write(f->volume_id, write_addr, memfs_ctx.log_buf, len);
    if (rc != len) {
        return -7;
    }

    ptr += len;

    log_head.ptr0 = ptr;
    log_head.ptr1 = ptr;
    log_head.ptr2 = ptr;

    rc = disk_write(f->volume_id, f->addr, &log_head, sizeof(log_head));
    if (rc != sizeof(log_head)) {
        return -8;
    }

    rc = disk_flush(f->volume_id);
    if (rc != 0) {
        return -9;
    }

    return 0;
}

int memfs_log_delete(const char* filename)
{
    // Если инициализация файловой системы прошла с ошибкой, работа невозможна
    if (!memfs_ctx.init_done) {
        return -10;
    }
    int fid = _memfs_get_fid_by_name(filename);
    memfs_file_t* f = _memfs_get_file_by_fid(fid);
    if (!f) {
        return -1;
    }

    if (f->attr != MEMFS_ATTR_LOG_FILE) {
        return -2;
    }

    if (!f->file_valid) {
        return -3;
    }

    int rc = disk_erase(f->volume_id, f->addr, f->cap);
    if (rc < 0) {
        return -6;
    }

    return 0;
}
