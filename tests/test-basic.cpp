#include <gtest/gtest.h>
#include "memfs.h"

memfs_file_t files[] = {
    { "/",          0,  0,  1024, MEMFS_ATTR_ROOT_FILE  },
    { "/bin/bl",    0,  0,  1024, MEMFS_ATTR_REGULAR    },
}

memfs_volume_t volumes[] = {
    { MEMFS_VOLUME_TYPE_RAM },
}

TEST(MemfsInit, input_check)
{
    int rc = 0;
    rc = memfs_init(files, sizeof(files), volumes, sizeof(volumes));
    EXPECT_EQ(rc, 0);

    rc = memfs_init(NULL, sizeof(files), volumes, sizeof(volumes));
    EXPECT_LT(rc, 0);

    rc = memfs_init(files, 0, volumes, sizeof(volumes));
    EXPECT_LT(rc, 0);

    rc = memfs_init(files, sizeof(files), NULL, sizeof(volumes));
    EXPECT_LT(rc, 0);

    rc = memfs_init(files, sizeof(files), volumes, 0);
    EXPECT_LT(rc, 0);
}