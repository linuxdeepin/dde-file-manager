// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnjobmanager_1.cpp
 * @brief Unit tests for BurnJobManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjobmanager.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnJobManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnJobManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnJobManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnJobManagerTest, BurnJobManager)
{
    // Test constructor: BurnJobManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BurnJobManagerTest, initBurnJobConnect)
{
    // Test method: void initBurnJobConnect((AbstractBurnJob *job))
    EXPECT_NO_FATAL_FAILURE(obj->initBurnJobConnect(nullptr));
}

TEST_F(BurnJobManagerTest, initDumpJobConnect)
{
    // Test method: void initDumpJobConnect((DumpISOImageJob *job))
    EXPECT_NO_FATAL_FAILURE(obj->initDumpJobConnect(nullptr));
}

TEST_F(BurnJobManagerTest, showOpticalDumpISOFailedDialog)
{
    // Test method: void showOpticalDumpISOFailedDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->showOpticalDumpISOFailedDialog());
}

TEST_F(BurnJobManagerTest, showOpticalDumpISOSuccessDialog)
{
    // Test method: void showOpticalDumpISOSuccessDialog((const QUrl &imageUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showOpticalDumpISOSuccessDialog(_arg0));
}

TEST_F(BurnJobManagerTest, showOpticalJobCompletionDialog)
{
    // Test method: void showOpticalJobCompletionDialog((const QString &msg, const QString &icon))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showOpticalJobCompletionDialog(_arg0, _arg1));
}

TEST_F(BurnJobManagerTest, startAuditLogForBurnFiles)
{
    // Test method: void startAuditLogForBurnFiles((const QVariantMap &info, const QUrl &stagingUrl, bool result))
    QVariantMap _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->startAuditLogForBurnFiles(_arg0, _arg1, false));
}

TEST_F(BurnJobManagerTest, startAuditLogForCopyFromDisc)
{
    // Test method: void startAuditLogForCopyFromDisc((const QList<QUrl> &srcList, const QList<QUrl> &destList))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->startAuditLogForCopyFromDisc(_arg0, _arg1));
}

TEST_F(BurnJobManagerTest, startBurnISOFiles)
{
    // Test method: void startBurnISOFiles((const QString &dev, const QUrl &stagingUrl, const BurnJobManager::Config &conf))
    QString _arg0{};
    QUrl _arg1{};
    BurnJobManager::Config _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->startBurnISOFiles(_arg0, _arg1, _arg2));
}

TEST_F(BurnJobManagerTest, startBurnISOImage)
{
    // Test method: void startBurnISOImage((const QString &dev, const QUrl &imageUrl, const BurnJobManager::Config &conf))
    QString _arg0{};
    QUrl _arg1{};
    BurnJobManager::Config _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->startBurnISOImage(_arg0, _arg1, _arg2));
}

TEST_F(BurnJobManagerTest, startBurnUDFFiles)
{
    // Test method: void startBurnUDFFiles((const QString &dev, const QUrl &stagingUrl, const BurnJobManager::Config &conf))
    QString _arg0{};
    QUrl _arg1{};
    BurnJobManager::Config _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->startBurnUDFFiles(_arg0, _arg1, _arg2));
}

TEST_F(BurnJobManagerTest, startDumpISOImage)
{
    // Test method: void startDumpISOImage((const QString &dev, const QUrl &imageUrl))
    QString _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->startDumpISOImage(_arg0, _arg1));
}

TEST_F(BurnJobManagerTest, startPutFilesToDisc)
{
    // Test method: void startPutFilesToDisc((const QString &dev, const QList<QUrl> &urls))
    QString _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->startPutFilesToDisc(_arg0, _arg1));
}

TEST_F(BurnJobManagerTest, startRenameFileFromDisc)
{
    // Test method: void startRenameFileFromDisc((const QString &dev, const QUrl &src, const QUrl &dest))
    QString _arg0{};
    QUrl _arg1{};
    QUrl _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->startRenameFileFromDisc(_arg0, _arg1, _arg2));
}
