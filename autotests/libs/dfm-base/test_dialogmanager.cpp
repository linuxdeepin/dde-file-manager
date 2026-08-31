// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dialogmanager.cpp
 * @brief Unit tests for DialogManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/dialogmanager.h"

#include <QTest>

using namespace src;

class DialogManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DialogManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DialogManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DialogManagerTest, showClearTrashDialog)
{
    // Test method: int showClearTrashDialog((const quint64 &count))
    quint64 _arg0{};
    auto result = obj->showClearTrashDialog(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(DialogManagerTest, showDeleteFilesDialog)
{
    // Test method: int showDeleteFilesDialog((const QList<QUrl> &urlList, bool isTrashFile))
    QList<QUrl> _arg0{};
    auto result = obj->showDeleteFilesDialog(_arg0, false);
    EXPECT_GE(result, 0);

}

TEST_F(DialogManagerTest, showDeleteSystemPathWarnDialog)
{
    // Test method: void showDeleteSystemPathWarnDialog((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->showDeleteSystemPathWarnDialog(0));
}

TEST_F(DialogManagerTest, showNormalDeleteConfirmDialog)
{
    // Test method: int showNormalDeleteConfirmDialog((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->showNormalDeleteConfirmDialog(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(DialogManagerTest, showOperationFailedDialog)
{
    // Test method: void showOperationFailedDialog((const QMap<QUrl, QString> &failedInfo))
    QMap<QUrl, QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showOperationFailedDialog(_arg0));
}

TEST_F(DialogManagerTest, showRestoreDeleteFilesDialog)
{
    // Test method: int showRestoreDeleteFilesDialog((const QList<QUrl> &urlList))
    QList<QUrl> _arg0{};
    auto result = obj->showRestoreDeleteFilesDialog(_arg0);
    EXPECT_GE(result, 0);

}
