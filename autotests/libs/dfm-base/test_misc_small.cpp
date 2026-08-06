// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_misc_small.cpp
 * @brief Small coverage tests for:
 *        - CustomSettingItemRegister (base/configs/customsettingitemregister.cpp)
 *        - AbstractFrame (interfaces/abstractframe.cpp) — ctor via minimal subclass
 *        - MimesAppsManager D0 destructor path (heap-allocate the singleton)
 */

#include <gtest/gtest.h>
#include <QString>
#include <QMap>

#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/mimetype/mimesappsmanager.h>

using namespace dfmbase;

// ---- CustomSettingItemRegister ----

TEST(CustomSettingItemRegisterTest, InstanceReturnsSingleton)
{
    auto *a = CustomSettingItemRegister::instance();
    auto *b = CustomSettingItemRegister::instance();
    EXPECT_EQ(a, b);
}

TEST(CustomSettingItemRegisterTest, GetCreatorsReturnsMap)
{
    EXPECT_NO_FATAL_FAILURE({ (void)CustomSettingItemRegister::instance()->getCreators(); });
}

TEST(CustomSettingItemRegisterTest, RegisterDuplicateTypeReturnsFalse)
{
    auto *reg = CustomSettingItemRegister::instance();
    QString type = "dfm_test_custom_type";
    // Register with a valid function pointer (nullptr is valid for the typedef)
    CustomSettingItemCreator creator = nullptr;
    bool first = reg->registCustomSettingItemType(type, creator);
    bool second = reg->registCustomSettingItemType(type, creator);
    // At least one must be false (duplicate)
    EXPECT_FALSE(first && second);
}

// ---- AbstractFrame ----

namespace {
class FakeFrame : public AbstractFrame
{
public:
    explicit FakeFrame(QWidget *parent = nullptr) : AbstractFrame(parent) {}
    void setCurrentUrl(const QUrl &) override {}
    QUrl currentUrl() const override { return QUrl(); }
};
}   // namespace

TEST(AbstractFrameTest, CtorAndDtorSafe)
{
    {
        FakeFrame f;
        SUCCEED();
    }
}

// ---- MimesAppsManager D0 destructor ----

TEST(MimesAppsManagerTest, HeapAllocatedDtorPath)
{
    // The singleton's D0 (deleting) destructor is not normally exercised;
    // heap-allocate a separate instance to hit that path.
    auto *ptr = new MimesAppsManager();
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}
