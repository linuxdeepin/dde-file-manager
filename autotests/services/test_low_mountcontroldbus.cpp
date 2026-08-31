// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_mountcontroldbus.cpp
 * @brief Unit tests for MountControlDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/mountcontrol/mountcontroldbus.h"

#include <QTest>

using namespace src;

class MountControlDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MountControlDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MountControlDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MountControlDBusTest, Mount)
{
    // Test method: QVariantMap Mount((const QString &path, const QVariantMap &opts))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->Mount(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MountControlDBusTest, SupportedFileSystems)
{
    // Test getter: QStringList SupportedFileSystems()
    auto result = obj->SupportedFileSystems();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MountControlDBusTest, Unmount)
{
    // Test method: QVariantMap Unmount((const QString &path, const QVariantMap &opts))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->Unmount(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MountControlDBusTest, MountControlDBus_Destructor)
{
    // Test method:  ~MountControlDBus(())
    EXPECT_NO_FATAL_FAILURE({ MountControlDBus *tmp = new MountControlDBus(); delete tmp; });
}
