// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfilehelper_1.cpp
 * @brief Unit tests for VaultFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultfilehelper.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultFileHelperTest, VaultFileHelper)
{
    // Test constructor: VaultFileHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultFileHelperTest, callBackFunction)
{
    // Test method: void callBackFunction((const AbstractJobHandler::CallbackArgus args))
    EXPECT_NO_FATAL_FAILURE(obj->callBackFunction(AbstractJobHandler::CallbackArgus()));
}

TEST_F(VaultFileHelperTest, copyFile)
{
    // Test method: bool copyFile((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags))
    auto result = obj->copyFile(0, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, cutFile)
{
    // Test method: bool cutFile((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags))
    auto result = obj->cutFile(0, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, handleFinishedNotify)
{
    // Test method: void handleFinishedNotify((const JobInfoPointer &jobInfo))
    JobInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFinishedNotify(_arg0));
}

TEST_F(VaultFileHelperTest, instance)
{
    // Test getter: DFMGLOBAL_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(VaultFileHelperTest, makeDir)
{
    // Test method: bool makeDir((const quint64 windowId, const QUrl url,
                              const QUrl &targetUrl,
                              const QVariant custom,
                              AbstractJobHandler::OperatorCallback callback))
    QUrl _arg2{};
    auto result = obj->makeDir(0, QUrl(), _arg2, QVariant(), AbstractJobHandler::OperatorCallback());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, moveToTrash)
{
    // Test method: bool moveToTrash((const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags))
    auto result = obj->moveToTrash(0, QList<QUrl>(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, openFileInPlugin)
{
    // Test method: bool openFileInPlugin((quint64 windowId, const QList<QUrl> urls))
    auto result = obj->openFileInPlugin(0, QList<QUrl>());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, renameFile)
{
    // Test method: bool renameFile((const quint64 windowId, const QUrl oldUrl, const QUrl newUrl, const AbstractJobHandler::JobFlags flags))
    auto result = obj->renameFile(0, QUrl(), QUrl(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, renameFiles)
{
    // Test method: bool renameFiles((const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> replacePair, bool flg))
    auto result = obj->renameFiles(0, QList<QUrl>(), QPair<QString, QString>(), false);
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, renameFilesAddText)
{
    // Test method: bool renameFilesAddText((const quint64 windowId, const QList<QUrl> urls, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> replacePair))
    auto result = obj->renameFilesAddText(0, QList<QUrl>(), QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag>());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, setPermision)
{
    // Test method: bool setPermision((const quint64 windowId,
                                   const QUrl url,
                                   const QFileDevice::Permissions permissions,
                                   bool *ok,
                                   QString *error))
    auto result = obj->setPermision(0, QUrl(), QFileDevice::Permissions(), nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, touchCustomFile)
{
    // Test method: bool touchCustomFile((const quint64 windowId, const QUrl url, const QUrl &targetUrl,
                                      const QUrl tempUrl, const QString &suffix,
                                      const QVariant &custom, AbstractJobHandler::OperatorCallback callback,
                                      QString *error))
    QUrl _arg2{};
    QString _arg4{};
    QVariant _arg5{};
    auto result = obj->touchCustomFile(0, QUrl(), _arg2, QUrl(), _arg4, _arg5, AbstractJobHandler::OperatorCallback(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, touchFile)
{
    // Test method: bool touchFile((const quint64 windowId,
                                const QUrl url, const QUrl &targetUrl,
                                const DFMGLOBAL_NAMESPACE::CreateFileType type,
                                const QString &suffix,
                                const QVariant &custom, AbstractJobHandler::OperatorCallback callback,
                                QString *error))
    QUrl _arg2{};
    QString _arg4{};
    QVariant _arg5{};
    auto result = obj->touchFile(0, QUrl(), _arg2, DFMGLOBAL_NAMESPACE::CreateFileType(), _arg4, _arg5, AbstractJobHandler::OperatorCallback(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, transUrlsToLocal)
{
    // Test method: QList<QUrl> transUrlsToLocal((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->transUrlsToLocal(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
