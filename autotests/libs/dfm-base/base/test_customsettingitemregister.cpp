// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"

#include <dfm-base/settingdialog/customsettingitemregister.h>

TEST(TestCustomSettingItemRegister, SingletonCreation_Basic)
{
    using namespace dfmbase;

    CustomSettingItemRegister *instance1 = CustomSettingItemRegister::instance();
    CustomSettingItemRegister *instance2 = CustomSettingItemRegister::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST(TestCustomSettingItemRegister, RegistCustomSettingItemType)
{
    using namespace dfmbase;

    CustomSettingItemRegister *registerer = CustomSettingItemRegister::instance();
    ASSERT_NE(registerer, nullptr);

    // Create a simple creator function - returns a pair of QWidget pointers
    CustomSettingItemCreator creator = [](QObject *parent) -> std::pair<QWidget *, QWidget *> {
        Q_UNUSED(parent);
        return { nullptr, nullptr };
    };

    // Test registering a new type
    const QString typeName = "test-custom-item";
    bool result = registerer->registCustomSettingItemType(typeName, creator);
    EXPECT_TRUE(result);

    // Test registering the same type again should fail
    bool resultAgain = registerer->registCustomSettingItemType(typeName, creator);
    EXPECT_FALSE(resultAgain);
}

TEST(TestCustomSettingItemRegister, GetCreators)
{
    using namespace dfmbase;

    CustomSettingItemRegister *registerer = CustomSettingItemRegister::instance();
    ASSERT_NE(registerer, nullptr);

    // Get creators before any registration
    const QMap<QString, CustomSettingItemCreator> &creatorsBefore = registerer->getCreators();

    // Create a simple creator function
    CustomSettingItemCreator creator = [](QObject *parent) -> std::pair<QWidget *, QWidget *> {
        Q_UNUSED(parent);
        return { nullptr, nullptr };
    };

    // Register a new type
    const QString typeName = "test-custom-widget";
    registerer->registCustomSettingItemType(typeName, creator);

    // Get creators after registration
    const QMap<QString, CustomSettingItemCreator> &creatorsAfter = registerer->getCreators();

    // Test that we have more creators after registration
    EXPECT_TRUE(creatorsAfter.size() >= creatorsBefore.size() + 1);
}
