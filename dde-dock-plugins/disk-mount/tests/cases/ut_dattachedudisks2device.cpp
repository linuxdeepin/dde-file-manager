#include "dattachedudisks2device.h"

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>


namespace  {
    class TestDAttachedUdisks2Device : public testing::Test {
    public:

        void SetUp() override
        {
           mDummyblkDevice = DDiskManager::createBlockDevice("/media/max");
           mUdisks2Device = new DAttachedUdisks2Device(mDummyblkDevice);
        }
        void TearDown() override
        {
            delete mDummyblkDevice;
            delete mUdisks2Device;
        }

    public:
        DBlockDevice* mDummyblkDevice = nullptr;
        DAttachedUdisks2Device* mUdisks2Device = nullptr;
    };
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_is_valid)
{
    EXPECT_TRUE(mUdisks2Device->isValid());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_not_detachable)
{
    EXPECT_FALSE(mUdisks2Device->detachable());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_name_contains_0B)
{
    QString name = mUdisks2Device->displayName();
    EXPECT_TRUE(name.contains("0 B"));
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_usage_not_valid)
{
    EXPECT_FALSE(mUdisks2Device->deviceUsageValid());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_get_default_iconname)
{
    QString name = mUdisks2Device->iconName();
    EXPECT_TRUE(name.contains("drive-harddisk"));
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_no_mounturl)
{
    QUrl url = mUdisks2Device->mountpointUrl();
    EXPECT_FALSE(url.isValid());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_sharedptr_not_null)
{
    ASSERT_TRUE(mUdisks2Device->blockDevice());
}
