// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/vaultvisiblemanager.h"
#include "utils/policy/policymanager.h"
#include "events/vaulteventcaller.h"

#include <gtest/gtest.h>

#include <dfm-framework/event/event.h>

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QUrl>
#include <QMap>

DPVAULT_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_VaultVisibleManager, isVaultEnabled)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::isVaultVisiable, []{
        return true;
    });

    bool isOk = VaultVisibleManager::instance()->isVaultEnabled();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, infoRegister)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::slotVaultPolicy, []{});
    stub.set_lamda(&VaultVisibleManager::isVaultEnabled, []{
        return true;
    });

    VaultVisibleManager::instance()->infoRegisterState = false;
    VaultVisibleManager::instance()->infoRegister();

    EXPECT_TRUE(VaultVisibleManager::instance()->infoRegisterState);
}

TEST(UT_VaultVisibleManager, pluginServiceRegister_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::isVaultEnabled, [ &isOk ]{
        isOk = true;
        return false;
    });

    VaultVisibleManager::instance()->pluginServiceRegister();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, pluginServiceRegister_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::isVaultEnabled, [ &isOk ]{
        isOk = true;
        return true;
    });

    VaultVisibleManager::instance()->pluginServiceRegister();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, addSideBarVaultItem)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::isVaultEnabled, []{
        return true;
    });
    typedef QVariant(EventChannelManager::*FuncType)(const QString &, const QString &, int, QUrl &&, QVariantMap &);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [ &isOk ]{
        isOk = true;
        return QVariant();
    });

    VaultVisibleManager::instance()->addVaultComputerMenu();
    VaultVisibleManager::instance()->addSideBarVaultItem();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, addComputer)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::isVaultEnabled, []{
        return true;
    });
    typedef QVariant(EventChannelManager::*FuncType)(const QString &, const QString &, QString, QUrl &&, int &&, bool &&);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [ &isOk ]{
        isOk = true;
        return QVariant();
    });

    VaultVisibleManager::instance()->addComputer();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, onWindowOpened_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [ &isOk ]{
        isOk = true;
        return nullptr;
    });

    VaultVisibleManager::instance()->onWindowOpened(0);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, onWindowOpened_two)
{
    bool isOk { false };
    FileManagerWindow window(QUrl("file:///UT_TEST"));

    stub_ext::StubExt stub;
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [ &window ]{
        return &window;
    });
    stub.set_lamda(&VaultVisibleManager::addSideBarVaultItem, []{});
    stub.set_lamda(&VaultVisibleManager::addComputer, []{});
    stub.set_lamda(&VaultEventCaller::sendBookMarkDisabled, [ &isOk ]{
        isOk = true;
    });

    VaultVisibleManager::instance()->onWindowOpened(0);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, removeSideBarVaultItem)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef QVariant(EventChannelManager::*FuncType)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [ &isOk ]{
        isOk = true;
        return QVariant();
    });

    VaultVisibleManager::instance()->removeSideBarVaultItem();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, removeComputerVaultItem)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef QVariant(EventChannelManager::*FuncType)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [ &isOk ]{
        isOk = true;
        return QVariant();
    });

    VaultVisibleManager::instance()->removeComputerVaultItem();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultVisibleManager, onComputerRefresh)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::addComputer, [ &isOk ]{
        isOk = true;
    });

    VaultVisibleManager::instance()->onComputerRefresh();

    EXPECT_TRUE(isOk);
}
