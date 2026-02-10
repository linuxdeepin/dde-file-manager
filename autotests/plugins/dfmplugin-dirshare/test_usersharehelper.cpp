// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "utils/usersharehelper.h"
#include "utils/sharewatchermanager.h"
#include "dfmplugin_dirshare_global.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-framework/event/event.h>

#include <QStandardPaths>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QSettings>
#include <QProcess>

#define DeclareDBusCallFunc_Full()                                         \
    typedef QDBusMessage (QDBusAbstractInterface::*Call)(const QString &,  \
                                                         const QVariant &, \
                                                         const QVariant &, \
                                                         const QVariant &, \
                                                         const QVariant &, \
                                                         const QVariant &, \
                                                         const QVariant &, \
                                                         const QVariant &, \
                                                         const QVariant &)

#define DeclareDBusCallFunc_Custom(params...) \
    typedef QDBusMessage (QDBusAbstractInterface::*Call)(params)

using namespace dfmplugin_dirshare;
DFMBASE_USE_NAMESPACE

class UT_UserShareHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_UserShareHelper, Instance)
{
    EXPECT_NO_FATAL_FAILURE(UserShareHelper::instance());
    EXPECT_EQ(UserShareHelper::instance(), UserShareHelper::instance());
}

TEST_F(UT_UserShareHelper, CanShare)
{
    EXPECT_FALSE(UserShareHelperInstance->canShare(nullptr));

    bool isProtocolFile = true;
    stub.set_lamda(&DeviceProxyManager::isFileOfProtocolMounts, [&] { __DBG_STUB_INVOKE__ return isProtocolFile; });
    auto info = InfoFactory::create<SyncFileInfo>(QUrl::fromLocalFile("/home"));
    EXPECT_TRUE(info);

    EXPECT_FALSE(UserShareHelperInstance->canShare(info));

    isProtocolFile = false;
    bool isBurnFile = true;
    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&] { __DBG_STUB_INVOKE__ return isBurnFile; });
    EXPECT_FALSE(UserShareHelperInstance->canShare(info));

    isBurnFile = false;
    EXPECT_TRUE(UserShareHelperInstance->canShare(info));
}

TEST_F(UT_UserShareHelper, NeedDisableShareWidget)
{
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->needDisableShareWidget(nullptr));
    EXPECT_FALSE(UserShareHelperInstance->needDisableShareWidget(nullptr));
}

TEST_F(UT_UserShareHelper, Share)
{
    bool serviceRunning = false;
    stub.set_lamda(&UserShareHelper::isSambaServiceRunning, [&] { __DBG_STUB_INVOKE__ return serviceRunning; });
    bool startSmbResult = false;
    stub.set_lamda(&UserShareHelper::startSambaServiceAsync, [&](void *, StartSambaFinished finished) {
        __DBG_STUB_INVOKE__
        serviceRunning = startSmbResult;
        if (finished) finished(startSmbResult, "");
    });

    EXPECT_FALSE(UserShareHelperInstance->share({}));

    startSmbResult = true;
    EXPECT_FALSE(UserShareHelperInstance->share({}));
    EXPECT_TRUE(serviceRunning);

    QString execPath;
    stub.set_lamda(&QStandardPaths::findExecutable, [&] { __DBG_STUB_INVOKE__ return execPath; });
    stub.set_lamda(&DialogManager::showErrorDialog, [] { __DBG_STUB_INVOKE__ });
    EXPECT_FALSE(UserShareHelperInstance->share({}));

    execPath = "this is where net command located";
    stub.set_lamda(&UserShareHelper::getOldShareByNewShare, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });
    bool isValidShare = false;
    stub.set_lamda(&UserShareHelper::isValidShare, [&] { __DBG_STUB_INVOKE__ return isValidShare; });
    EXPECT_TRUE(UserShareHelperInstance->share({}));

    isValidShare = true;
    QVariantMap shareInfo { { ShareInfoKeys::kName, "-" } };
    EXPECT_FALSE(UserShareHelperInstance->share(shareInfo));

    shareInfo.insert(ShareInfoKeys::kName, "hello");
    shareInfo.insert(ShareInfoKeys::kWritable, true);
    shareInfo.insert(ShareInfoKeys::kAnonymous, true);
    stub.set_lamda(&UserShareHelper::readPort, [] { __DBG_STUB_INVOKE__ return 139; });
    int cmdResult = -1;
    stub.set_lamda(&UserShareHelper::runNetCmd, [&] { __DBG_STUB_INVOKE__ return cmdResult; });
    stub.set_lamda(&UserShareHelper::removeShareByPath, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_FALSE(UserShareHelperInstance->share(shareInfo));

    cmdResult = 0;
    EXPECT_TRUE(UserShareHelperInstance->share(shareInfo));
}

TEST_F(UT_UserShareHelper, IsUserSharepasswordSet)
{
    DeclareDBusCallFunc_Custom(const QString &, const QString &);
    auto call = static_cast<Call>(&QDBusAbstractInterface::call);
    stub.set_lamda(call, [] { __DBG_STUB_INVOKE__ return QDBusMessage(); });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->isUserSharePasswordSet("test"));
}

// TEST_F(UT_UserShareHelper, SetSambaPasswd)
// {
//     DeclareDBusCallFunc_Custom(const QString &, const QString &, const QString &);
//     auto call = static_cast<Call>(&QDBusAbstractInterface::call);
//     stub.set_lamda(call, [] { __DBG_STUB_INVOKE__ return QDBusMessage(); });
//     EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->setSambaPasswd("test", "test"));
// }

TEST_F(UT_UserShareHelper, RemoveShareByPath)
{
    QString shareName;
    stub.set_lamda(&UserShareHelper::shareNameByPath, [&] { __DBG_STUB_INVOKE__ return shareName; });
    stub.set_lamda(&UserShareHelper::removeShareByShareName, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->removeShareByPath("/"));
}

TEST_F(UT_UserShareHelper, ReadPort)
{
    int port = 0;
    stub.set_lamda(static_cast<QVariant (QSettings::*)(QAnyStringView) const>(&QSettings::value), [&] { __DBG_STUB_INVOKE__ return QVariant(port); });
    EXPECT_EQ(0, UserShareHelperInstance->readPort());
    port = 100;
    EXPECT_EQ(port, UserShareHelperInstance->readPort());
}

TEST_F(UT_UserShareHelper, ShareInfos)
{
    EXPECT_EQ(UserShareHelperInstance->sharedInfos.count(), UserShareHelperInstance->shareInfos().count());
}

TEST_F(UT_UserShareHelper, ShareInfoByPath)
{
    stub.set_lamda(&UserShareHelper::shareNameByPath, [] { __DBG_STUB_INVOKE__ return ""; });
    stub.set_lamda(&UserShareHelper::shareInfoByShareName, [] { __DBG_STUB_INVOKE__ return QVariantMap { { ShareInfoKeys::kName, "hello" } }; });
    EXPECT_TRUE(UserShareHelperInstance->shareInfoByPath("/").value(ShareInfoKeys::kName).toString() == "hello");
}

TEST_F(UT_UserShareHelper, ShareInfoByShareName)
{
    EXPECT_TRUE(UserShareHelperInstance->shareInfoByShareName("").isEmpty());
    UserShareHelperInstance->sharedInfos.insert("hello", QVariantMap { { ShareInfoKeys::kName, "hello" } });
    EXPECT_TRUE(UserShareHelperInstance->shareInfoByShareName("hello").value(ShareInfoKeys::kName).toString() == "hello");
    UserShareHelperInstance->sharedInfos.clear();
}

TEST_F(UT_UserShareHelper, ShareNameByPath)
{
    EXPECT_TRUE(UserShareHelperInstance->shareNameByPath("/").isEmpty());

    UserShareHelperInstance->sharePathToShareName.insert("/", QStringList { "hello", "world" });
    EXPECT_EQ("world", UserShareHelperInstance->shareNameByPath("/"));
    UserShareHelperInstance->sharePathToShareName.clear();
}

TEST_F(UT_UserShareHelper, WhoShared)
{
    stub.set_lamda(&QFileInfo::ownerId, [] { __DBG_STUB_INVOKE__ return 1000; });
    EXPECT_EQ(1000, UserShareHelperInstance->whoShared("hello"));
}

TEST_F(UT_UserShareHelper, IsShared)
{
    UserShareHelperInstance->sharePathToShareName.insert("/", QStringList { "hello", "world" });
    EXPECT_TRUE(UserShareHelperInstance->isShared("/"));
    EXPECT_FALSE(UserShareHelperInstance->isShared("/home"));
    UserShareHelperInstance->sharePathToShareName.clear();
}

TEST_F(UT_UserShareHelper, CurrentUserName)
{
    EXPECT_TRUE(!UserShareHelperInstance->currentUserName().isEmpty());
}

TEST_F(UT_UserShareHelper, IsSambaServiceRunning)
{
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->isSambaServiceRunning());
}

TEST_F(UT_UserShareHelper, SharedIP)
{
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->sharedIP());
    EXPECT_FALSE(UserShareHelperInstance->sharedIP().isEmpty());
}

TEST_F(UT_UserShareHelper, HandleSetPassword)
{
    stub.set_lamda(&UserShareHelper::setSambaPasswd, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->handleSetPassword("1234"));
}

TEST_F(UT_UserShareHelper, ReadShareInfos)
{
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->readShareInfos(false));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->readShareInfos(true));
}

TEST_F(UT_UserShareHelper, OnShareChanged)
{
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->onShareChanged("/"));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->onShareChanged("/:tmp"));
}

TEST_F(UT_UserShareHelper, OnShareFileDeleted)
{
    stub.set_lamda(&UserShareHelper::onShareChanged, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&UserShareHelper::removeShareWhenShareFolderDeleted, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->onShareFileDeleted("/var/lib/samba/usershares/hello"));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->onShareFileDeleted("/home/test"));
}

TEST_F(UT_UserShareHelper, OnShareMoved)
{
    stub.set_lamda(&UserShareHelper::onShareFileDeleted, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&UserShareHelper::onShareChanged, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->onShareMoved("a", "b"));
}

TEST_F(UT_UserShareHelper, InitConnect)
{
    delete UserShareHelperInstance->pollingSharesTimer;
    UserShareHelperInstance->pollingSharesTimer = nullptr;
    EXPECT_TRUE(UserShareHelperInstance->pollingSharesTimer == nullptr);

    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->initConnect());
    EXPECT_TRUE(UserShareHelperInstance->pollingSharesTimer);
}

TEST_F(UT_UserShareHelper, InitMonitorPath)
{
    ShareInfoList lst;
    lst.append(QVariantMap { {} });
    stub.set_lamda(&UserShareHelper::shareInfos, [&] { __DBG_STUB_INVOKE__ return lst; });
    stub.set_lamda(&ShareWatcherManager::add, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->initMonitorPath());
}

TEST_F(UT_UserShareHelper, RemoveShareWhenShareFolderDeleted)
{
    QString name;
    stub.set_lamda(&UserShareHelper::shareNameByPath, [&] { __DBG_STUB_INVOKE__ return name; });
    stub.set_lamda(&UserShareHelper::removeShareByShareName, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->removeShareWhenShareFolderDeleted("/"));

    name = "hello";
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->removeShareWhenShareFolderDeleted("/"));
}

TEST_F(UT_UserShareHelper, GetOldShareByNewShare)
{
    UserShareHelperInstance->sharePathToShareName.insert("hello", QStringList { "a", "B" });
    UserShareHelperInstance->sharePathToShareName.insert("world", QStringList { "c", "d" });

    stub.set_lamda(&UserShareHelper::shareInfoByShareName, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->getOldShareByNewShare(QVariantMap { { ShareInfoKeys::kName, "hello" },
                                                                                         { ShareInfoKeys::kPath, "world" } }));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->getOldShareByNewShare(QVariantMap {}));

    UserShareHelperInstance->sharePathToShareName.clear();
}

TEST_F(UT_UserShareHelper, RunNetCmd) { }

TEST_F(UT_UserShareHelper, HandleErrorWhenShareFailed)
{
    stub.set_lamda(&DialogManager::showErrorDialog, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->handleErrorWhenShareFailed(0, ""));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->handleErrorWhenShareFailed(0, "is already a valid system user name"));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->handleErrorWhenShareFailed(0, "as we are restricted to only sharing directories we own."));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->handleErrorWhenShareFailed(0, "contains invalid characters"));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->handleErrorWhenShareFailed(0, "net usershare add: failed to add share Error was"));
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->handleErrorWhenShareFailed(0, "gethostname failed  net usershare add: cannot convert name"));
}

TEST_F(UT_UserShareHelper, MakeInfoByFileContent)
{
    QMap<QString, QString> fileContent { { "sharename", "hello" },
                                         { "path", "/home" },
                                         { "usershare_acl", "acl" } };
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->makeInfoByFileContent(fileContent));
    QVariantMap share = UserShareHelperInstance->makeInfoByFileContent(fileContent);
    EXPECT_TRUE(share.value(ShareInfoKeys::kName, "").toString() == "hello");
}

TEST_F(UT_UserShareHelper, ValidShareInfoCount)
{
    stub.set_lamda(&UserShareHelper::isValidShare, [] { __DBG_STUB_INVOKE__ return true; });
    UserShareHelperInstance->sharedInfos.clear();
    UserShareHelperInstance->sharedInfos.insert("hello", QVariantMap());
    EXPECT_EQ(1, UserShareHelperInstance->validShareInfoCount());
    UserShareHelperInstance->sharedInfos.clear();
}

TEST_F(UT_UserShareHelper, StartSmbService) { }

TEST_F(UT_UserShareHelper, SetSmbdAutoStart)
{
    DeclareDBusCallFunc_Custom(const QString &);
    auto call = static_cast<Call>(&QDBusAbstractInterface::call);
    stub.set_lamda(call, [] { __DBG_STUB_INVOKE__ return QDBusMessage(); });
}

TEST_F(UT_UserShareHelper, IsValidShare)
{
    EXPECT_TRUE(UserShareHelperInstance->isValidShare(QVariantMap { { ShareInfoKeys::kName, "hello" },
                                                                    { ShareInfoKeys::kPath, "/" } }));

    EXPECT_FALSE(UserShareHelperInstance->isValidShare(QVariantMap { { ShareInfoKeys::kName, "world" },
                                                                     { ShareInfoKeys::kPath, "/where/not/exists" } }));
}

TEST_F(UT_UserShareHelper, EmitShareCountChanged)
{
    typedef bool (dpf::EventDispatcherManager::*Publish)(const QString &, const QString &, int);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->emitShareCountChanged(10));
}

TEST_F(UT_UserShareHelper, EmitShareAdded)
{
    typedef bool (dpf::EventDispatcherManager::*Publish)(const QString &, const QString &, QString);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->emitShareAdded("/home/xxxx"));
}

TEST_F(UT_UserShareHelper, EmitShareRemoved)
{
    typedef bool (dpf::EventDispatcherManager::*Publish)(const QString &, const QString &, QString);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(UserShareHelperInstance->emitShareAdded("/home/xxxx"));
}
