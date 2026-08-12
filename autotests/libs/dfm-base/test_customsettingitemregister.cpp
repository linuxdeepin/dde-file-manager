// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customsettingitemregister.cpp
 * @brief Unit tests for CustomSettingItemRegister (customsettingitemregister.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>

#include <QWidget>
#include <QObject>
#include <QString>

using namespace dfmbase;

static QPair<QWidget *, QWidget *> utDummyCreator(QObject *parent)
{
    Q_UNUSED(parent)
    return { nullptr, nullptr };
}

static QString utUniqueType(const char *tag)
{
    static int counter = 0;
    return QStringLiteral("ut-csir-%1-%2").arg(QLatin1String(tag)).arg(counter++);
}

TEST(CustomSettingItemRegisterTest, InstanceReturnsNonNullSingleton)
{
    auto *a = CustomSettingItemRegister::instance();
    auto *b = CustomSettingItemRegister::instance();
    EXPECT_EQ(a, b);
}

TEST(CustomSettingItemRegisterTest, RegisterNewTypeSucceeds)
{
    auto *r = CustomSettingItemRegister::instance();
    QString type = utUniqueType("new");
    EXPECT_TRUE(r->registCustomSettingItemType(type, utDummyCreator));
    EXPECT_TRUE(r->getCreators().contains(type));
}

TEST(CustomSettingItemRegisterTest, RegisterDuplicateReturnsFalse)
{
    auto *r = CustomSettingItemRegister::instance();
    QString type = utUniqueType("dup");
    ASSERT_TRUE(r->registCustomSettingItemType(type, utDummyCreator));
    EXPECT_FALSE(r->registCustomSettingItemType(type, utDummyCreator));
}

TEST(CustomSettingItemRegisterTest, GetCreatorsGrowsOnNewRegistration)
{
    auto *r = CustomSettingItemRegister::instance();
    auto before = r->getCreators().size();
    QString t1 = utUniqueType("acc1");
    QString t2 = utUniqueType("acc2");
    ASSERT_TRUE(r->registCustomSettingItemType(t1, utDummyCreator));
    ASSERT_TRUE(r->registCustomSettingItemType(t2, utDummyCreator));
    EXPECT_EQ(r->getCreators().size(), before + 2);
}
