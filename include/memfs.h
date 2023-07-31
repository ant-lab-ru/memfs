#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "memfs-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEMFS_BINARY_FILE_WITHOUT_HEADER UINT32_MAX

typedef enum {
    MEMFS_VOLUME_TYPE_RAM = 0, // Запись и чтение по байтам
} memfs_volume_type_t;

typedef struct {
    memfs_volume_type_t type;
} memfs_volume_t;

typedef enum {
    MEMFS_MODE_R = 0,// Только чтение
    MEMFS_MODE_W,    // Только запись, при открытии содержимое файла удаляется
    MEMFS_MODE_A,    // Только запись, данные добавляются в конец файла
    MEMFS_MODE_RW,   // Запись и чтение, при открытии содержимое файла удаляется
    MEMFS_MODE_RA,   // Запись и чтение, данные добавляются в конец файла
} memfs_mode_t;

typedef enum {
    MEMFS_ATTR_REGULAR = 0,    
	MEMFS_ATTR_BINARY,
    MEMFS_ATTR_LOG_FILE,
	MEMFS_ATTR_READ_ONLY,
    MEMFS_ATTR_ROOT_FILE,
} memfs_file_attr_t;

typedef struct {
    uint32_t file_crc;
    uint32_t file_size;
} memfs_binary_file_header_t;

typedef struct {
    uint32_t file_crc;
    uint32_t file_size;
    char     file_name[MEMFS_FILE_NAME_MAX_LEN];
} memfs_file_header_t;

typedef enum {
    MEMFS_FILE_CLOSED = 0,
    MEMFS_FILE_STATE_RO,
    MEMFS_FILE_STATE_WO,
    MEMFS_FILE_STATE_RW,
} memfs_file_state_t;

typedef struct {
    // Имя файла. Не более 31 символа
    const char* name;
    // Индекс области памяти
    const uint8_t volume_id;
    // Адрес начала файла в байтах
    const uint32_t addr;
    // Размер выделенного под файл. Размер данных равен cap - sizeof(emfs_file_header_t)
    const uint32_t cap;
    // Аттрибут файла
    const memfs_file_attr_t attr;
    // Только для EMFS_ATTR_BINARY
    const uint32_t binary_file_header_offset;

    /* Private state */
    memfs_file_state_t state;
    bool write_allowed;
    bool file_valid;
    uint32_t read_ptr;
    uint32_t size;
    uint32_t crc;
} memfs_file_t;

typedef struct {
    bool file_valid;
    uint32_t expected_crc;
    uint32_t calculated_crc;
    uint32_t size;
    uint32_t addr;
    uint32_t cap;
    uint8_t volume_id;
    char file_name[MEMFS_FILE_NAME_MAX_LEN];
} memfs_stat_t;

/* Init memfs */
int memfs_init(memfs_file_t* files, const uint16_t files_count, memfs_volume_t* volumes, const uint8_t volumes_count);

/* Std functions */
int memfs_open(const char* filename, int mode);
int memfs_close(int fid);
int memfs_read(int fid, uint8_t* buffer, uint32_t length);
int memfs_write(int fid, uint8_t* data, uint32_t size);
int memfs_seek(int fid, uint32_t offset);

/* High reliability functions */
int memfs_unlock_file_write(const char* filename);
int memfs_lock_file_write(const char* filename);
int memfs_stat(const char* filename, memfs_stat_t* stat);

/* Log functions */
int memfs_log_write(const char* filename, const char* format, ...);
int memfs_log_delete(const char* filename);

#ifdef __cplusplus
}
#endif

