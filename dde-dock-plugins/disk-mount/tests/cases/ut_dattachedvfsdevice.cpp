#include "dattachedvfsdevice.h"

#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>


namespace  {
    class TestDAttachedVfsDevice : public testing::Test {
    public:

        void SetUp() override
        {
            mDummyVfsDevice.reset( new DAttachedVfsDevice("/data/home/max/uos.iso"));
        }
        void TearDown() override
        {
        }

    public:
        std::shared_ptr<DAttachedVfsDevice> mDummyVfsDevice = nullptr;
    };
}

TEST_F(TestDAttachedVfsDevice, dummy_device_cant_mounted)
{
    EXPECT_FALSE(mDummyVfsDevice->isValid());
}
