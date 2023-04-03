// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "addr_any.h"
#include "plugins/filemanager/core/dfmplugin-computer/controller/computercontroller.h"
#include "plugins/filemanager/core/dfmplugin-computer/utils/computerutils.h"
#include "plugins/filemanager/core/dfmplugin-computer/events/computereventcaller.h"
#include "plugins/filemanager/core/dfmplugin-computer/menu/computermenuscene.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QProcess>
#include <QMenu>

#include <gtest/gtest.h>

DPCOMPUTER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_ComputerController : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    ComputerController *ins { ComputerControllerInstance };

    const QUrl blockUrl = QUrl("entry:///sda.blockdev");
    const QUrl appUrl = QUrl("entry:///baidu.appentry");
    const QUrl protoUrl = QUrl("entry:///hello.protodev");
    const QUrl protoStashedUrl = QUrl("entry:///hello.protodevstashed");
    const QUrl vaultUrl = QUrl("entry:///vault.vault");
};

TEST_F(UT_ComputerController, OnOpenItem)
{
    {
        stub_ext::StubExt s;
        s.set_lamda(ComputerEventCaller::sendOpenItem, [] { __DBG_STUB_INVOKE__ });
        EXPECT_NO_FATAL_FAILURE(ins->onOpenItem(0, vaultUrl));
    }

    {
        stub_ext::StubExt s;
        s.set_lamda(&QVariant::toBool, [] { __DBG_STUB_INVOKE__ return false; });
        s.set_lamda(&EntryFileInfo::isAccessable, [] { __DBG_STUB_INVOKE__ return false; });
        s.set_lamda(&ComputerController::handleUnAccessableDevCdCall, [] { __DBG_STUB_INVOKE__ });
        EXPECT_NO_FATAL_FAILURE(ins->onOpenItem(0, blockUrl));
    }

    QUrl targetUrl;
    stub.set_lamda(&EntryFileInfo::targetUrl, [&targetUrl] { __DBG_STUB_INVOKE__ return targetUrl; });
    typedef void (*Cd)(quint64, const QUrl &);
    stub.set_lamda(static_cast<Cd>(ComputerEventCaller::cdTo), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&QVariant::toBool, [] { __DBG_STUB_INVOKE__ return true; });   // isOptical

    targetUrl = QUrl::fromLocalFile("/home");
    {
        stub_ext::StubExt s;
        s.set_lamda(&ComputerUtils::makeBurnUrl, [&targetUrl] { __DBG_STUB_INVOKE__ return targetUrl; });
        bool isSmbOrFtp = false;
        s.set_lamda(DeviceUtils::isSamba, [&isSmbOrFtp] { __DBG_STUB_INVOKE__ return isSmbOrFtp; });
        s.set_lamda(DeviceUtils::isFtp, [&isSmbOrFtp] { __DBG_STUB_INVOKE__ return isSmbOrFtp; });
        s.set_lamda(&ComputerController::handleNetworkCdCall, [] { __DBG_STUB_INVOKE__ });
        EXPECT_NO_FATAL_FAILURE(ins->onOpenItem(0, blockUrl));

        isSmbOrFtp = true;
        EXPECT_NO_FATAL_FAILURE(ins->onOpenItem(0, blockUrl));
    }

    targetUrl = QUrl();
    {
        stub_ext::StubExt s;
        typedef void (ComputerController::*MountDev)(quint64, const DFMEntryFileInfoPointer, ComputerController::ActionAfterMount);
        s.set_lamda(static_cast<MountDev>(&ComputerController::mountDevice), [] { __DBG_STUB_INVOKE__ });
        s.set_lamda(&ComputerController::actMount, [] { __DBG_STUB_INVOKE__ });
        typedef bool (*StartDetached)(const QString &);
        s.set_lamda(static_cast<StartDetached>(QProcess::startDetached), [] { __DBG_STUB_INVOKE__ return true; });

        EXPECT_NO_FATAL_FAILURE(ins->onOpenItem(0, blockUrl));
        EXPECT_NO_FATAL_FAILURE(ins->onOpenItem(0, protoStashedUrl));
        EXPECT_NO_FATAL_FAILURE(ins->onOpenItem(0, appUrl));
    }
}

static inline DFMBASE_NAMESPACE::AbstractMenuScene *stubCreateComputerScene(const QString &)
{
    __DBG_STUB_INVOKE__
    return new ComputerMenuScene;
}
TEST_F(UT_ComputerController, OnMenuRequest)
{
    {
        ComputerUtils::contextMenuEnabled = false;
        EXPECT_NO_FATAL_FAILURE(ins->onMenuRequest(0, blockUrl, false));
    }

    ::AddrAny any;
    std::map<std::string, void *> ret;
    any.get_local_func_addr_symtab("^dfmplugin_menu_util::menuSceneCreateScene(QString const&)", ret);
    if (ret.size() == 0)
        return;
    stub.set(ret.at("dfmplugin_menu_util::menuSceneCreateScene(QString const&)"), stubCreateComputerScene);

    ComputerUtils::contextMenuEnabled = true;
    bool initMenu = false;
    stub.set_lamda(VADDR(ComputerMenuScene, initialize), [&initMenu] { __DBG_STUB_INVOKE__ return initMenu; });
    stub.set_lamda(VADDR(ComputerMenuScene, create), [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(VADDR(ComputerMenuScene, updateState), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(ComputerMenuScene, triggered), [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(ins->onMenuRequest(0, blockUrl, false));

    initMenu = true;
    QAction *act = new QAction;
    typedef QAction *(QMenu::*Exec)(const QPoint &, QAction *);
    stub.set_lamda(static_cast<Exec>(&QMenu::exec), [act] { __DBG_STUB_INVOKE__ return act; });
    EXPECT_NO_FATAL_FAILURE(ins->onMenuRequest(0, blockUrl, false));
}

TEST_F(UT_ComputerController, DoRename)
{
    bool removable = true;
    stub.set_lamda(&QVariant::toBool, [&removable] { __DBG_STUB_INVOKE__ return removable; });
    QString name;
    stub.set_lamda(&EntryFileInfo::displayName, [&name] { __DBG_STUB_INVOKE__ return name; });
    EXPECT_NO_FATAL_FAILURE(ins->doRename(0, blockUrl, name));

    stub.set_lamda(ComputerUtils::getBlockDevIdByUrl, [] { __DBG_STUB_INVOKE__ return ""; });
    stub.set_lamda(&DeviceManager::renameBlockDevAsync, [](void *, const QString &, const QString &, const QVariantMap &, CallbackType2 cb) {
        if (cb) cb(false, DFMMOUNT::OperationErrorInfo());
    });

    name = "hello";
    EXPECT_NO_FATAL_FAILURE(ins->doRename(0, blockUrl, "world"));

    removable = false;
    stub.set_lamda(&ComputerController::doSetAlias, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->doRename(0, blockUrl, "world"));
}

TEST_F(UT_ComputerController, DoSetAlias)
{
    //    EXPECT_NO_FATAL_FAILURE(ins->doSetAlias(nullptr, "hello"));
    //    DFMEntryFileInfoPointer info(new EntryFileInfo(blockUrl));
    //    EXPECT_NO_FATAL_FAILURE(ins->doSetAlias(info, "world"));

    //    QString uuid;
    //    stub.set_lamda(&QVariant::toString, [&uuid]{ __DBG_STUB_INVOKE__ return uuid; });
    //    typedef QVariant (Settings::*Value)(const QString&, const QString &, const QVariant &) const;
    //    stub.set_lamda(static_cast<Value>(&Settings::value), []{ __DBG_STUB_INVOKE__
    //                QVariantMap map { {"alias", "root"}, {"name", "system disk"}, {"uuid", "thisIsUUID"}};
    //        QVariantList ret;
    //        ret << map;
    //        return ret;
    //    });
    //    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    //    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), []{ __DBG_STUB_INVOKE__ });
    //    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QUrl, QVariantMap &&);
    //    stub.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), []{ __DBG_STUB_INVOKE__ return QVariant(); });

    //    uuid = "not existed uuid";
    //    EXPECT_NO_FATAL_FAILURE(ins->doSetAlias(info, "not exist"));
    //    uuid = "thisIsUUID";
    //    EXPECT_NO_FATAL_FAILURE(ins->doSetAlias(info, ""));
    //    EXPECT_NO_FATAL_FAILURE(ins->doSetAlias(info, "newName"));
}

TEST_F(UT_ComputerController, MountDevice)
{
    //    EXPECT_NO_FATAL_FAILURE(ins->mountDevice(0, nullptr));

    //    using namespace GlobalServerDefines::DeviceProperty;
    //    QVariantMap properties {
    //        {kIsEncrypted, true},
    //        {kCleartextDevice, "12345"},
    //        {kHasFileSystem, ""},
    //        {kOpticalDrive, ""},
    //        {kOptical, true}
    //    };
    //    stub.set_lamda(&EntryFileInfo::extraProperties, [&properties]{
    //        __DBG_STUB_INVOKE__
    //                return properties;
    //    });
}
TEST_F(UT_ComputerController, ActEject) { }
TEST_F(UT_ComputerController, ActOpenInNewWindow) { }
TEST_F(UT_ComputerController, ActOpenInNewTab) { }
TEST_F(UT_ComputerController, ActMount) { }
TEST_F(UT_ComputerController, ActUnmount) { }
TEST_F(UT_ComputerController, ActSafelyRemove) { }
TEST_F(UT_ComputerController, ActRename) { }
TEST_F(UT_ComputerController, ActFormat) { }
TEST_F(UT_ComputerController, ActRemove) { }
TEST_F(UT_ComputerController, ActProperties) { }
TEST_F(UT_ComputerController, ActLogoutAndForgetPasswd) { }
TEST_F(UT_ComputerController, ActErase) { }
TEST_F(UT_ComputerController, WaitUDisks2DataReady) { }
