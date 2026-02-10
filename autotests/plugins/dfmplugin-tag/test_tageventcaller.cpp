// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/events/tageventcaller.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QRect>

using namespace dfmplugin_tag;
DPF_USE_NAMESPACE

Q_DECLARE_METATYPE(QRectF *)
Q_DECLARE_METATYPE(QPoint *)

TEST(UT_TagEventCaller, sendOpenWindow)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, QUrl);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [&isCall] {
        isCall = true;
        return true;
    });

    TagEventCaller::sendOpenWindow(QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_TagEventCaller, sendOpenTab)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [&isCall] {
        isCall = true;
        return true;
    });

    TagEventCaller::sendOpenTab(0, QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_TagEventCaller, sendCheckTabAddable)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return true;
    });
    TagEventCaller::sendCheckTabAddable(0);

    EXPECT_TRUE(isCall);
}

TEST(UT_TagEventCaller, getCollectionView)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, QString);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return QVariant();
    });

    TagEventCaller::getCollectionView("");

    EXPECT_TRUE(isCall);
}

TEST(UT_TagEventCaller, sendOpenFiles)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, quint64, const QList<QUrl> &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [&isCall] {
        isCall = true;
        return true;
    });

    TagEventCaller::sendOpenFiles(0, QList<QUrl>() << QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_TagEventCaller, getCollectionVisualRect)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, QString, const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return QVariant();
    });
    TagEventCaller::getCollectionVisualRect("", QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_TagEventCaller, getCollectionIconRect)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, QString, QRect &);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return QVariant();
    });
    TagEventCaller::getCollectionIconRect("", QRect());

    EXPECT_TRUE(isCall);
}
