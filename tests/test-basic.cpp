#include <gtest/gtest.h>
#include "memfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GTEST_COUT std::cerr << "[          ] INFO "

memfs_file_t files[] = {
    { "/",              0,  0,  1024, MEMFS_ATTR_ROOT_FILE  },
    { "/usr/file1",     0,  0,  1024, MEMFS_ATTR_REGULAR    },
    { "/bin/bl",        0,  1024,  1024, MEMFS_ATTR_REGULAR    },
};
#define FILES_COUNT (sizeof(files)/sizeof(memfs_file_t))

memfs_volume_t volumes[] = {
    { MEMFS_VOLUME_TYPE_RAM },
};
#define VOLUMES_COUNT (sizeof(volumes)/sizeof(memfs_volume_t))

uint8_t test_buf_data[1 * 1024 * 1024] = {0};

int disk_erase(uint8_t volume, uint32_t addr, uint32_t size) {
    return 0;
}

int disk_read(uint8_t volume, uint32_t addr, uint8_t* buffer, uint32_t length) {
    memcpy(buffer, &test_buf_data[addr], length);
    return length;
}

int disk_write(uint8_t volume, uint32_t addr, uint8_t* data, uint32_t size) {
    memcpy(&test_buf_data[addr], data, size);
    return size;
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

TEST(MemfsWriteRead, file_write_read)
{
    EXPECT_EQ(memfs_init(files, FILES_COUNT, volumes, VOLUMES_COUNT), 0);
    GTEST_COUT << "memfs_init done" << std::endl;

    EXPECT_EQ(memfs_unlock_file_write("/usr/file1"), 0);

    int rc = memfs_open("/usr/file1", MEMFS_MODE_W);
    EXPECT_GE(rc, 0);

    char test_str[] = "Hello, this is test string!";
    EXPECT_EQ(memfs_write(rc, (uint8_t*)test_str, sizeof(test_str)), sizeof(test_str));

    EXPECT_EQ(memfs_close(rc), 0);
    GTEST_COUT << "write done" << std::endl;

    rc = memfs_open("/usr/file1", MEMFS_MODE_R);
    EXPECT_GE(rc, 0);

    char test_read_str[sizeof(test_str)] = {0};
    EXPECT_EQ(memfs_read(rc, (uint8_t*)test_read_str, sizeof(test_str)), sizeof(test_str));

    for (int i = 0; i < sizeof(test_str); i++){
        EXPECT_EQ(test_read_str[i], test_str[i]);
    }

    EXPECT_EQ(memfs_close(rc), 0);
    GTEST_COUT << "read done" << std::endl;
}