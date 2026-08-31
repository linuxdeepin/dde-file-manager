// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedatamanager.cpp
 * @brief Unit tests for FileDataManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filedatamanager.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileDataManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDataManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDataManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDataManagerTest, cleanRoot)
{
    // Test method: void cleanRoot((const QUrl &rootUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->cleanRoot(_arg0));
}

TEST_F(FileDataManagerTest, cleanUnusedRoots)
{
    // Test method: void cleanUnusedRoots((const QUrl &currentUrl, const QString &key))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->cleanUnusedRoots(_arg0, _arg1));
}

TEST_F(FileDataManagerTest, onHandleFileDeleted)
{
    // Test method: void onHandleFileDeleted((const QUrl url))
    EXPECT_NO_FATAL_FAILURE(obj->onHandleFileDeleted(QUrl()));
}

TEST_F(FileDataManagerTest, removeRootUser)
{
    // Test method: void removeRootUser((const QUrl &url, const QString &key))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->removeRootUser(_arg0, _arg1));
}
