// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "utils/computerutils.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QIcon>
#include <QUrl>
#include <QWidget>
#include <QMimeDatabase>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerUtils : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }
    static void SetUpTestCase()
    {
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
    }

private:
    stub_ext::StubExt stub;
    const QUrl blockUrl = QUrl("entry:///sda.blockdev");
    const QUrl appUrl = QUrl("entry:///baidu.appentry");
    const QUrl protoUrl = QUrl("entry:///hello.protodev");
    const QUrl protoStashedUrl = QUrl("entry:///hello.protodevstashed");
    const QUrl vaultUrl = QUrl("entry:///vault.vault");
};

TEST_F(UT_ComputerUtils, Scheme)
{
    EXPECT_TRUE(ComputerUtils::scheme() == "computer");
}

TEST_F(UT_ComputerUtils, Icon)
{
    EXPECT_NO_FATAL_FAILURE(ComputerUtils::icon().themeName());
}

TEST_F(UT_ComputerUtils, RootUrl)
{
    EXPECT_TRUE(ComputerUtils::rootUrl().scheme() == "computer");
    EXPECT_TRUE(ComputerUtils::rootUrl().path() == "/");
}

TEST_F(UT_ComputerUtils, MenuSceneName)
{
    EXPECT_TRUE(ComputerUtils::menuSceneName() == "ComputerMenu");
}

TEST_F(UT_ComputerUtils, GetWinId)
{
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [] { __DBG_STUB_INVOKE__ return 0; });
    EXPECT_EQ(0, ComputerUtils::getWinId(nullptr));
    EXPECT_NO_FATAL_FAILURE(ComputerUtils::getWinId(nullptr));
}

TEST_F(UT_ComputerUtils, MakeBlockDevUrl)
{
    QUrl u;
    EXPECT_NO_FATAL_FAILURE(u = ComputerUtils::makeBlockDevUrl("/org/freedesktop/UDisks2/block_devices/sdb1"));
    EXPECT_TRUE(u.scheme() == "entry");
    EXPECT_TRUE(u.path() == "sdb1.blockdev");
}

TEST_F(UT_ComputerUtils, GetBlockDevIdByUrl)
{
    EXPECT_TRUE(ComputerUtils::getBlockDevIdByUrl(QUrl("file:///")) == "");
    EXPECT_TRUE(ComputerUtils::getBlockDevIdByUrl(QUrl("entry:///hello")) == "");
    EXPECT_TRUE(ComputerUtils::getBlockDevIdByUrl(QUrl("entry:sdb1.blockdev")) == "/org/freedesktop/UDisks2/block_devices/sdb1");
}

TEST_F(UT_ComputerUtils, MakeProtocolDevUrl)
{
    QUrl u;
    EXPECT_NO_FATAL_FAILURE(u = ComputerUtils::makeProtocolDevUrl("smb://1.2.3.4/hello"));
    EXPECT_TRUE(u.path().startsWith("smb://1.2.3.4/hello"));
    EXPECT_TRUE(u.path().endsWith("protodev"));
    EXPECT_TRUE(u.scheme() == "entry");
}

TEST_F(UT_ComputerUtils, GetProtocolDevIdByUrl)
{
    EXPECT_TRUE(ComputerUtils::getProtocolDevIdByUrl(QUrl::fromLocalFile("/home")) == "");
    EXPECT_TRUE(ComputerUtils::getProtocolDevIdByUrl(blockUrl) == "");
}

TEST_F(UT_ComputerUtils, MakeAppEntryUrl)
{
    EXPECT_NO_FATAL_FAILURE(ComputerUtils::makeAppEntryUrl("hello").isValid());
    EXPECT_NO_FATAL_FAILURE(ComputerUtils::makeAppEntryUrl("/usr/share/dde-file-manager/extensions/appEntry/.readme").isValid());
    EXPECT_NO_FATAL_FAILURE(ComputerUtils::makeAppEntryUrl("/usr/share/dde-file-manager/extensions/appEntry/readme.desktop").isValid());
}

TEST_F(UT_ComputerUtils, GetAppEntryFileUrl)
{
    EXPECT_FALSE(ComputerUtils::getAppEntryFileUrl(QUrl::fromLocalFile("/home")).isValid());
    EXPECT_FALSE(ComputerUtils::getAppEntryFileUrl(QUrl()).isValid());
    EXPECT_TRUE(ComputerUtils::getAppEntryFileUrl(appUrl).isValid());
    EXPECT_TRUE(ComputerUtils::getAppEntryFileUrl(appUrl).path().endsWith("desktop"));
}

TEST_F(UT_ComputerUtils, MakeLocalUrl)
{
    EXPECT_TRUE(ComputerUtils::makeLocalUrl("/home").scheme() == "file");
    EXPECT_TRUE(ComputerUtils::makeLocalUrl("/home").path() == "/home");
}

TEST_F(UT_ComputerUtils, MakeBurnUrl)
{
    EXPECT_TRUE(ComputerUtils::makeBurnUrl("/dev/sr0").scheme() == "burn");
    EXPECT_TRUE(ComputerUtils::makeBurnUrl("/dev/sr1").path() == "/dev/sr1/disc_files/");
}

TEST_F(UT_ComputerUtils, IsPresetSuffix)
{
    EXPECT_TRUE(ComputerUtils::isPresetSuffix(SuffixInfo::kBlock));
    EXPECT_TRUE(ComputerUtils::isPresetSuffix(SuffixInfo::kProtocol));
    EXPECT_TRUE(ComputerUtils::isPresetSuffix(SuffixInfo::kUserDir));
    EXPECT_TRUE(ComputerUtils::isPresetSuffix(SuffixInfo::kAppEntry));
    EXPECT_FALSE(ComputerUtils::isPresetSuffix("hello"));
    EXPECT_FALSE(ComputerUtils::isPresetSuffix("vault"));
    EXPECT_FALSE(ComputerUtils::isPresetSuffix("balabala"));
    EXPECT_FALSE(ComputerUtils::isPresetSuffix(""));
}

TEST_F(UT_ComputerUtils, ShouldSystemPartitionHide)
{
    stub.set_lamda(&QVariant::toBool, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ComputerUtils::shouldSystemPartitionHide());
}

TEST_F(UT_ComputerUtils, ShouldLoopPartitionsHide)
{
    stub.set_lamda(&QVariant::toBool, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ComputerUtils::shouldSystemPartitionHide());
}

TEST_F(UT_ComputerUtils, SortItem)
{
    EXPECT_FALSE(ComputerUtils::sortItem(QUrl::fromLocalFile("/home"), QUrl::fromLocalFile("/")));
    EXPECT_FALSE(ComputerUtils::sortItem(appUrl, blockUrl));
    EXPECT_TRUE(ComputerUtils::sortItem(blockUrl, appUrl));
    DFMEntryFileInfoPointer infa(new EntryFileInfo(blockUrl));
    DFMEntryFileInfoPointer infb(new EntryFileInfo(blockUrl));
    stub.set_lamda(&EntryFileInfo::displayName, [&](void *me) {
        __DBG_STUB_INVOKE__
                if (me == infa.data())
                return "a";
        return "b";
    });
    EXPECT_TRUE(ComputerUtils::sortItem(infa, infb));
}

TEST_F(UT_ComputerUtils, DeviceTypeInfo)
{
    DFMEntryFileInfoPointer inf(new EntryFileInfo(blockUrl));
    QList<AbstractEntryFileEntity::EntryOrder> orders { AbstractEntryFileEntity::kOrderUserDir, AbstractEntryFileEntity::kOrderSysDiskRoot,
                                                        AbstractEntryFileEntity::kOrderRemovableDisks, AbstractEntryFileEntity::kOrderOptical,
                                                        AbstractEntryFileEntity::kOrderMTP,
                                                        AbstractEntryFileEntity::kOrderGPhoto2, AbstractEntryFileEntity::kOrderFiles };
    stub.set_lamda(&EntryFileInfo::order, [&] { __DBG_STUB_INVOKE__ return orders.takeFirst(); });
    for (int i = 0; i < orders.count(); i++) {
        EXPECT_FALSE(ComputerUtils::deviceTypeInfo(inf).isEmpty());
    }
}

TEST_F(UT_ComputerUtils, DeviceProtpertyDialog)
{
    EXPECT_FALSE(ComputerUtils::devicePropertyDialog(QUrl::fromLocalFile("/home")));
    QWidget *w { nullptr };
    EXPECT_NO_FATAL_FAILURE(w = ComputerUtils::devicePropertyDialog(blockUrl));
    EXPECT_TRUE(w);
    delete w;
}

TEST_F(UT_ComputerUtils, ConvertToDevUrl)
{
    EXPECT_TRUE(blockUrl == ComputerUtils::convertToDevUrl(blockUrl));
}

TEST_F(UT_ComputerUtils, GetUniqueInteger)
{
    QSet<int> rets;
    for (int i = 0; i < 100; i++)
        EXPECT_NO_FATAL_FAILURE(rets << ComputerUtils::getUniqueInteger());
    EXPECT_TRUE(rets.count() == 100);
}

// TEST_F(UT_ComputerUtils, CheckGvfsMountExist){}
TEST_F(UT_ComputerUtils, SetCursorState)
{
    EXPECT_NO_FATAL_FAILURE(ComputerUtils::setCursorState());
    EXPECT_NO_FATAL_FAILURE(ComputerUtils::setCursorState(true));
}

TEST_F(UT_ComputerUtils, AllSystemUUIDs)
{
    QList<QStringList> devs {
        { "sys1", "sys2", "loop1", "loop2" },
        { "loop1", "loop2" }
    };
    stub.set_lamda(&DeviceProxyManager::getAllBlockIds, [&] { __DBG_STUB_INVOKE__ return devs.takeFirst(); });
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [] { __DBG_STUB_INVOKE__ return QVariantMap { { GlobalServerDefines::DeviceProperty::kUUID, "ssss" } }; });
    EXPECT_FALSE(ComputerUtils::allValidBlockUUIDs().isEmpty());
}

TEST_F(UT_ComputerUtils, SystemBlkDevUrlByUUIDs)
{
    stub.set_lamda(&DeviceProxyManager::getAllBlockIdsByUUID, [] { __DBG_STUB_INVOKE__
                                                                           return QStringList{"/org/freedesktop/UDisks2/block_devices/sda1", "/org/freedesktop/UDisks2/block_devices/sda2"}; });
    EXPECT_TRUE(ComputerUtils::blkDevUrlByUUIDs({}).count() >= 0);
}
