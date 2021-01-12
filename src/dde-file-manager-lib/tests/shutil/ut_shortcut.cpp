#include "shutil/shortcut.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <memory>

namespace  {
    class TestShortcut : public testing::Test {
    public:
        void SetUp() override
        {
           mShortcut.reset(new Shortcut());
        }
        void TearDown() override
        {
        }

    public:
            std::shared_ptr<Shortcut> mShortcut = nullptr;
    };
}

TEST_F(TestShortcut, can_transferred_to_string)
{
    QString value = mShortcut->toStr();
    EXPECT_TRUE( value.contains("Ctrl + Shift + Tab"));
    EXPECT_TRUE( value.contains("Ctrl + W"));
    EXPECT_TRUE( value.contains("Ctrl + C"));
    EXPECT_TRUE( value.contains("Ctrl + V"));
    EXPECT_TRUE( value.contains("Shift + Left"));
    EXPECT_TRUE( value.contains("Ctrl + A"));
    EXPECT_TRUE( value.contains("Shift + Delete"));
}
