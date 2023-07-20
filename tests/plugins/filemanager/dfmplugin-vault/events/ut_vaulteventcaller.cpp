// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "events/vaulteventcaller.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

TEST(UT_VaultEventCaller, sendItemActived)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [ &isCall ]{
        isCall = true;
        return true;
    });

    VaultEventCaller::sendItemActived(0, QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_VaultEventCaller, sendOpenWindow)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, QUrl);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [ &isCall ]{
        isCall = true;
        return true;
    });

    VaultEventCaller::sendOpenWindow(QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_VaultEventCaller, sendOpenTab)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [ &isCall ]{
        isCall = true;
        return true;
    });

    VaultEventCaller::sendOpenTab(0, QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_VaultEventCaller, sendVaultProperty)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef QVariant(EventChannelManager::*FuncType)(const QString &, const QString &, QList<QUrl>, QVariantHash&&);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [ &isCall ]{
        isCall = true;
        return QVariant();
    });

    VaultEventCaller::sendVaultProperty(QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_VaultEventCaller, sendBookMarkDisabled)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef QVariant(EventChannelManager::*FuncType)(const QString &, const QString &, const QString);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [ &isCall ]{
        isCall = true;
        return QVariant();
    });

    VaultEventCaller::sendBookMarkDisabled("");

    EXPECT_TRUE(isCall);
}

TEST(UT_VaultEventCaller, sendOpenFiles)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, quint64, const QList<QUrl> &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [ &isCall ]{
        isCall = true;
        return true;
    });

    VaultEventCaller::sendOpenFiles(0, QList<QUrl>() << QUrl());

    EXPECT_TRUE(isCall);
}
