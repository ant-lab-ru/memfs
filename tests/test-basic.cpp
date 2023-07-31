#include <gtest/gtest.h>
#include "memfs.h"

#ifdef __cplusplus
extern "C" {
#endif

memfs_file_t files[] = {
    { "/",          0,  0,  1024, MEMFS_ATTR_ROOT_FILE  },
    { "/bin/bl",    0,  0,  1024, MEMFS_ATTR_REGULAR    },
};
#define FILES_COUNT (sizeof(files)/sizeof(memfs_file_t))

memfs_volume_t volumes[] = {
    { MEMFS_VOLUME_TYPE_RAM },
};
#define VOLUMES_COUNT (sizeof(volumes)/sizeof(memfs_volume_t))

int disk_erase(uint8_t volume, uint32_t addr, uint32_t size) {
    return 0;
}

int disk_read(uint8_t volume, uint32_t addr, uint8_t* buffer, uint32_t length) {
    return 0;
}

int disk_write(uint8_t volume, uint32_t addr, uint8_t* data, uint32_t size) {
    return 0;
}

int disk_flush(uint8_t volume) {
    return 0;
}
#ifdef __cplusplus
}
#endif

TEST(MemfsInit, input_check)
{
    int rc = 0;
    rc = memfs_init(files, FILES_COUNT, volumes, VOLUMES_COUNT);
    EXPECT_EQ(rc, 0);

    rc = memfs_init(NULL, FILES_COUNT, volumes, VOLUMES_COUNT);
    EXPECT_LT(rc, 0);

    rc = memfs_init(files, 0, volumes, VOLUMES_COUNT);
    EXPECT_LT(rc, 0);

    rc = memfs_init(files, FILES_COUNT, NULL, VOLUMES_COUNT);
    EXPECT_LT(rc, 0);

    rc = memfs_init(files, FILES_COUNT, volumes, 0);
    EXPECT_LT(rc, 0);
}