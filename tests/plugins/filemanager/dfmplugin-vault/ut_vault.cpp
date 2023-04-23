// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#include "stubext.h"
#include "vault.h"
#include "utils/vaultvisiblemanager.h"
#include "events/vaulteventreceiver.h"

#include <gtest/gtest.h>

#include <dfm-framework/event/eventdispatcher.h>

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

DPVAULT_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_Vault, Initialize)
{
    bool isRegister { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::infoRegister, [ &isRegister ] { __DBG_STUB_INVOKE__ isRegister = true; });
    stub.set_lamda(&VaultEventReceiver::connectEvent, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&VaultVisibleManager::pluginServiceRegister, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&Vault::bindWindows, []() { __DBG_STUB_INVOKE__ });

    Vault ins;
    ins.initialize();
    EXPECT_TRUE(isRegister);
}

TEST(UT_Vault, Start)
{
    stub_ext::StubExt stub;
    typedef void (VaultVisibleManager::*Func1)();
    typedef bool (EventDispatcherManager::*Subscribe1)(const QString &, const QString &, VaultVisibleManager *, Func1);
    stub.set_lamda(static_cast<Subscribe1>(&EventDispatcherManager::subscribe), [] { __DBG_STUB_INVOKE__ return false; });

    Vault ins;
    EXPECT_TRUE(ins.start());
}

TEST(UT_Vault, binWindows)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::onWindowOpened, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&FileManagerWindowsManager::windowIdList, [] { __DBG_STUB_INVOKE__ return QList<quint64> {1, 2}; });

    Vault ins;
    ins.bindWindows();
    bool bDisConnect = QObject::disconnect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened,
                        VaultVisibleManager::instance(), &VaultVisibleManager::onWindowOpened);
    EXPECT_TRUE(bDisConnect);
}
