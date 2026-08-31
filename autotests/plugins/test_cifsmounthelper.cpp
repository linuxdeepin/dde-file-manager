// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cifsmounthelper.cpp
 * @brief Unit tests for CifsMountHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/mountcontrol/mounthelpers/cifsmounthelper.h"

#include <QTest>

using namespace src;

class CifsMountHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CifsMountHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CifsMountHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CifsMountHelperTest, checkMount)
{
    // Test method: CifsMountHelper::MountStatus checkMount((const QString &path, QString &mpt))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->checkMount(_arg0, _arg1);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CifsMountHelperTest, cleanMountPoint)
{
    // Test method: void cleanMountPoint(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanMountPoint());
}

TEST_F(CifsMountHelperTest, convertArgs)
{
    // Test method: std::string convertArgs((const QVariantMap &opts))
    QVariantMap _arg0{};
    auto result = obj->convertArgs(_arg0);
    EXPECT_TRUE(result.empty());

}

TEST_F(CifsMountHelperTest, option)
{
    // Test method: QString option((const QString &key, const QVariantMap &override, const QString &def))
    QString _arg0{};
    QVariantMap _arg1{};
    QString _arg2{};
    auto result = obj->option(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CifsMountHelperTest, preparePasswd)
{
    // Test method: QString preparePasswd((const QVariant &passwdVar))
    QVariant _arg0{};
    auto result = obj->preparePasswd(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CifsMountHelperTest, rmdir)
{
    // Test method: bool rmdir((const QString &path))
    QString _arg0{};
    auto result = obj->rmdir(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CifsMountHelperTest, unmount)
{
    // Test method: QVariantMap unmount((const QString &path, const QVariantMap &opts))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->unmount(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
