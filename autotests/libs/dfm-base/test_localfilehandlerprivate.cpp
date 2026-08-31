// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfilehandlerprivate.cpp
 * @brief Unit tests for LocalFileHandlerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/localfilehandler.h"

#include <QTest>

using namespace src;

class LocalFileHandlerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LocalFileHandlerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LocalFileHandlerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LocalFileHandlerPrivateTest, handleExecutableFile)
{
    // Test method: bool handleExecutableFile((const QUrl &fileUrl, bool *result))
    QUrl _arg0{};
    auto result = obj->handleExecutableFile(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerPrivateTest, isFileRunnable)
{
    // Test method: bool isFileRunnable((const QString &path))
    QString _arg0{};
    auto result = obj->isFileRunnable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerPrivateTest, openExcutableFile)
{
    // Test method: bool openExcutableFile((const QString &path, int flag))
    QString _arg0{};
    auto result = obj->openExcutableFile(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerPrivateTest, openExcutableScriptFile)
{
    // Test method: bool openExcutableScriptFile((const QString &path, int flag))
    QString _arg0{};
    auto result = obj->openExcutableScriptFile(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerPrivateTest, shouldAskUserToAddExecutableFlag)
{
    // Test method: bool shouldAskUserToAddExecutableFlag((const QString &path))
    QString _arg0{};
    auto result = obj->shouldAskUserToAddExecutableFlag(_arg0);
    EXPECT_FALSE(result);

}
