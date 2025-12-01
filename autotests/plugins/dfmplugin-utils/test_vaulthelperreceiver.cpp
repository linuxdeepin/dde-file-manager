// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/vaultassist/vaulthelperreceiver.h"
#include "plugins/common/dfmplugin-utils/vaultassist/vaultassitcontrol.h"

#include <dfm-framework/event/event.h>

#include <QApplication>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_VaultHelperReceiver : public testing::Test
{
protected:
    void SetUp() override
    {
        receiver = new VaultHelperReceiver();
    }

    void TearDown() override
    {
        delete receiver;
        receiver = nullptr;
        stub.clear();
    }

    VaultHelperReceiver *receiver { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_VaultHelperReceiver, Constructor_CreatesReceiver)
{
    EXPECT_NE(receiver, nullptr);
}

TEST_F(UT_VaultHelperReceiver, initEventConnect_FollowsHook)
{
    bool followCalled = false;

    typedef bool (EventSequenceManager::*Follow)(const QString &, const QString &, VaultHelperReceiver *, decltype(&VaultHelperReceiver::handlemoveToTrash));
    stub.set_lamda(static_cast<Follow>(&EventSequenceManager::follow),
                   [&followCalled] {
                       __DBG_STUB_INVOKE__
                       followCalled = true;
                       return true;
                   });

    receiver->initEventConnect();

    EXPECT_TRUE(followCalled);
}

TEST_F(UT_VaultHelperReceiver, handlemoveToTrash_EmptySource_ReturnsFalse)
{
    bool result = receiver->handlemoveToTrash(0, QList<QUrl>(), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(result);
}

TEST_F(UT_VaultHelperReceiver, handlemoveToTrash_NonVaultFile_ReturnsFalse)
{
    stub.set_lamda(ADDR(VaultAssitControl, isVaultFile),
                   [](VaultAssitControl *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };

    bool result = receiver->handlemoveToTrash(0, urls, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(result);
}

TEST_F(UT_VaultHelperReceiver, handlemoveToTrash_VaultFile_ReturnsTrue)
{
    stub.set_lamda(ADDR(VaultAssitControl, isVaultFile),
                   [](VaultAssitControl *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(VaultAssitControl, transUrlsToLocal),
                   [](VaultAssitControl *, const QList<QUrl> &urls) -> QList<QUrl> {
                       __DBG_STUB_INVOKE__
                       return urls;
                   });

    typedef bool (EventDispatcherManager::*Publish)(EventType, quint64, QList<QUrl> &, AbstractJobHandler::JobFlags &,
                                                    std::nullptr_t &&, QVariant &&, AbstractJobHandler::OperatorCallback &);
    stub.set_lamda(static_cast<Publish>(&EventDispatcherManager::publish),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/test.txt");
    QList<QUrl> urls = { vaultUrl };

    bool result = receiver->handlemoveToTrash(0, urls, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(result);
}

TEST_F(UT_VaultHelperReceiver, handleFinishedNotify_RestoresCursor)
{
    bool restoreCalled = false;

    stub.set_lamda(&QApplication::restoreOverrideCursor,
                   [&restoreCalled]() {
                       __DBG_STUB_INVOKE__
                       restoreCalled = true;
                   });

    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    receiver->handleFinishedNotify(jobInfo);

    EXPECT_TRUE(restoreCalled);
}
