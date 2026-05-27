// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "interface/fileinfomodelshell.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QModelIndex>
#include <QAbstractItemModel>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_FileInfoModelShell : public testing::Test
{
protected:
    void SetUp() override
    {
        shell = new FileInfoModelShell();
    }

    void TearDown() override
    {
        delete shell;
        shell = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    FileInfoModelShell *shell = nullptr;
};

TEST_F(UT_FileInfoModelShell, initialize_SubscribesSignal_ReturnsTrue)
{
    bool result = shell->initialize();
    EXPECT_TRUE(result);
}

TEST_F(UT_FileInfoModelShell, sourceModel_FirstCall_GetsFromChannel)
{
    QAbstractItemModel *expectedModel = nullptr;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedModel]() {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(expectedModel);
    });

    QAbstractItemModel *result = shell->sourceModel();
    EXPECT_EQ(result, expectedModel);
}

TEST_F(UT_FileInfoModelShell, rootUrl_CallsChannel_ReturnsUrl)
{
    QUrl expectedUrl("file:///home/desktop");
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedUrl]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedUrl);
    });

    QUrl result = shell->rootUrl();
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(UT_FileInfoModelShell, rootIndex_CallsChannel_ReturnsIndex)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), []() {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(QModelIndex());
    });

    QModelIndex result = shell->rootIndex();
    EXPECT_FALSE(result.isValid());
}

TEST_F(UT_FileInfoModelShell, index_WithValidUrl_ReturnsIndex)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const QUrl &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), []() {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(QModelIndex());
    });

    QUrl url("file:///home/test/file.txt");
    QModelIndex result = shell->index(url);
    EXPECT_FALSE(result.isValid());
}

TEST_F(UT_FileInfoModelShell, fileInfo_CallsChannel_ReturnsInfo)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const QModelIndex &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), []() {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(FileInfoPointer());
    });

    QModelIndex index;
    FileInfoPointer result = shell->fileInfo(index);
    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_FileInfoModelShell, fileUrl_CallsChannel_ReturnsUrl)
{
    QUrl expectedUrl("file:///home/test/file.txt");
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QModelIndex);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedUrl]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedUrl);
    });

    QModelIndex index;
    QUrl result = shell->fileUrl(index);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(UT_FileInfoModelShell, files_CallsChannel_ReturnsUrls)
{
    QList<QUrl> expectedFiles = {
        QUrl("file:///file1.txt"),
        QUrl("file:///file2.txt")
    };
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedFiles]() {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(expectedFiles);
    });

    QList<QUrl> result = shell->files();
    EXPECT_EQ(result.size(), 2);
}

TEST_F(UT_FileInfoModelShell, refresh_CallsChannel_NoReturn)
{
    bool channelCalled = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QModelIndex);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&channelCalled]() {
        __DBG_STUB_INVOKE__
        channelCalled = true;
        return QVariant();
    });

    QModelIndex parent;
    shell->refresh(parent);
    EXPECT_TRUE(channelCalled);
}

TEST_F(UT_FileInfoModelShell, modelState_CallsChannel_ReturnsState)
{
    int expectedState = 1;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [expectedState]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedState);
    });

    int result = shell->modelState();
    EXPECT_EQ(result, expectedState);
}

