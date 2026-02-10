// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/global/globaleventreceiver.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-framework/dpf.h>

#include <QProcess>
#include <QUrl>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_GlobalEventReceiver : public testing::Test
{
protected:
    void SetUp() override
    {
        receiver = new GlobalEventReceiver();
    }

    void TearDown() override
    {
        delete receiver;
        receiver = nullptr;
        stub.clear();
    }

    GlobalEventReceiver *receiver { nullptr };
    stub_ext::StubExt stub;
};

// ========== initEventConnect 测试 ==========

TEST_F(UT_GlobalEventReceiver, initEventConnect_SubscribesEvent)
{
    bool subscribed = false;

    typedef bool (EventDispatcherManager::*Subscribe)(EventType, GlobalEventReceiver *, decltype(&GlobalEventReceiver::handleOpenAsAdmin));
    auto subscribe = static_cast<Subscribe>(&dpf::EventDispatcherManager::subscribe);
    stub.set_lamda(subscribe,
                   [&subscribed] {
                       __DBG_STUB_INVOKE__
                       subscribed = true;
                       return true;
                   });

    receiver->initEventConnect();

    EXPECT_TRUE(subscribed);
}

// ========== handleOpenAsAdmin 测试 ==========

TEST_F(UT_GlobalEventReceiver, handleOpenAsAdmin_InvalidUrl_ReturnsEarly)
{
    bool processStarted = false;

    using StartDetachedFunc = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFunc>(QProcess::startDetached),
                   [&processStarted] {
                       __DBG_STUB_INVOKE__
                       processStarted = true;
                       return true;
                   });

    receiver->handleOpenAsAdmin(QUrl());
    EXPECT_FALSE(processStarted);

    receiver->handleOpenAsAdmin(QUrl(""));
    EXPECT_FALSE(processStarted);
}

TEST_F(UT_GlobalEventReceiver, handleOpenAsAdmin_ValidLocalFile_StartsProcess)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test_file.txt");
    QString capturedPath;
    bool processStarted = false;

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    using StartDetachedFunc = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFunc>(QProcess::startDetached),
                   [&processStarted, &capturedPath](const QString &program, const QStringList &args, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       processStarted = true;
                       if (!args.isEmpty())
                           capturedPath = args[0];
                       return program == "dde-file-manager-pkexec";
                   });

    receiver->handleOpenAsAdmin(url);

    EXPECT_TRUE(processStarted);
    EXPECT_EQ(capturedPath, url.toLocalFile());
}

TEST_F(UT_GlobalEventReceiver, handleOpenAsAdmin_DirSymlink_UsesRedirectedPath)
{
    QUrl url = QUrl::fromLocalFile("/tmp/symlink_dir");
    QUrl redirectedUrl = QUrl::fromLocalFile("/tmp/real_dir");
    QString capturedPath;

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return type == OptInfoType::kIsDir || type == OptInfoType::kIsSymLink;
                   });

    stub.set_lamda(VADDR(FileInfo, urlOf),
                   [&redirectedUrl](FileInfo *, const UrlInfoType) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return redirectedUrl;
                   });

    using StartDetachedFunc = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFunc>(QProcess::startDetached),
                   [&capturedPath](const QString &, const QStringList &args, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       if (!args.isEmpty())
                           capturedPath = args[0];
                       return true;
                   });

    receiver->handleOpenAsAdmin(url);

    EXPECT_EQ(capturedPath, redirectedUrl.toLocalFile());
}

TEST_F(UT_GlobalEventReceiver, handleOpenAsAdmin_NonLocalUrl_UsesUrlString)
{
    QUrl url("smb://server/share");
    QString capturedPath;

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    using StartDetachedFunc = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFunc>(QProcess::startDetached),
                   [&capturedPath](const QString &, const QStringList &args, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       if (!args.isEmpty())
                           capturedPath = args[0];
                       return true;
                   });

    receiver->handleOpenAsAdmin(url);

    EXPECT_EQ(capturedPath, url.toString());
}

TEST_F(UT_GlobalEventReceiver, handleOpenAsAdmin_NullFileInfo_ContinuesExecution)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test_file.txt");
    bool processStarted = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    using StartDetachedFunc = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFunc>(QProcess::startDetached),
                   [&processStarted] {
                       __DBG_STUB_INVOKE__
                       processStarted = true;
                       return true;
                   });

    receiver->handleOpenAsAdmin(url);

    EXPECT_TRUE(processStarted);
}
