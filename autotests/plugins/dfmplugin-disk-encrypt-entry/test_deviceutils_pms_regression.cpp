// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for dfmplugin-disk-encrypt-entry (device_utils entry path).

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <QString>

using namespace dfmplugin_diskenc;

class DeviceUtilsPmsRegressionTest : public testing::Test
{
public:
    void SetUp() override { }
    void TearDown() override { }
};

// PMS:374085 resolveEntryBlockDevPath built "/sys/class/block/<name>/holders"
// paths from raw user input: empty names produced bogus lookups and names like
// "../../etc" could traverse outside the block sysfs dir. The fix rejects
// empty/traversal inputs and, in the fallback branch, encodes the device name
// before appending ".blockdev".
TEST_F(DeviceUtilsPmsRegressionTest, BUG374085_ResolveEntryBlockDevPath_RejectsInvalidAndEncodesFallback)
{
    // Empty device must be rejected (empty result), never resolved.
    EXPECT_TRUE(device_utils::resolveEntryBlockDevPath(QString()).isEmpty());

    // Path traversal payloads must be rejected outright.
    EXPECT_TRUE(device_utils::resolveEntryBlockDevPath("../../etc/passwd").isEmpty());
    EXPECT_TRUE(device_utils::resolveEntryBlockDevPath("/dev/../../etc/passwd").isEmpty());
    EXPECT_TRUE(device_utils::resolveEntryBlockDevPath("/dev/nvme..0p1").isEmpty());

    // Valid device without a resolvable block monitor falls back to encoding
    // the plain device name; alphanumeric names stay unchanged.
    EXPECT_EQ(device_utils::resolveEntryBlockDevPath("/dev/utnvme0p5"),
              QString("utnvme0p5.blockdev"));
}
