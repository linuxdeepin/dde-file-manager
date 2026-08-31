// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_applaunchutilsprivate.cpp
 * @brief Unit tests for AppLaunchUtilsPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/applaunchutils.h"

#include <QTest>

using namespace src;

class AppLaunchUtilsPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppLaunchUtilsPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppLaunchUtilsPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppLaunchUtilsPrivateTest, executeCommand)
{
    // Test method: bool executeCommand((const QString &program, const QStringList &args,
                                           const QString &type, const QString &workdir,
                                           const QString &runId, const QStringMap &envVars))
    QString _arg0{};
    QStringList _arg1{};
    QString _arg2{};
    QString _arg3{};
    QString _arg4{};
    QStringMap _arg5{};
    auto result = obj->executeCommand(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
    EXPECT_FALSE(result);

}

TEST_F(AppLaunchUtilsPrivateTest, launchByGio)
{
    // Test method: bool launchByGio((const QString &desktopFile, const QStringList &filePaths))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->launchByGio(_arg0, _arg1);
    EXPECT_FALSE(result);

}
