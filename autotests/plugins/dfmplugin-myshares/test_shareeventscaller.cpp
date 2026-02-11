// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "events/shareeventscaller.h"
#include "dfmplugin_myshares_global.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QList>
#include <QVariantHash>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

        class UT_ShareEventsCaller : public testing::Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

using namespace dfmplugin_myshares;
using namespace dpf;
DFMBASE_USE_NAMESPACE

TEST_F(UT_ShareEventsCaller, SendOpenDirs)
{
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, {}, ShareEventsCaller::OpenMode::kOpenInCurrentWindow));

    typedef bool (EventDispatcherManager::*Publish1)(int, QUrl);
    typedef bool (EventDispatcherManager::*Publish2)(int, quint64, QUrl &);

    auto publish1 = static_cast<Publish1>(&EventDispatcherManager::publish);
    stub.set_lamda(publish1, [] { __DBG_STUB_INVOKE__ return true; });
    auto publish2 = static_cast<Publish2>(&EventDispatcherManager::publish);
    stub.set_lamda(publish2, [] { __DBG_STUB_INVOKE__ return true; });

    QList<QUrl> urls { QUrl("usershare:///test") };
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInCurrentWindow));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInNewWindow));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInNewTab));

    urls << QUrl("usershare:///hello");
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInNewTab));
}

TEST_F(UT_ShareEventsCaller, SendCancelSharing)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl()));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl("file:///")));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl("usershare:///hello")));
}

TEST_F(UT_ShareEventsCaller, SendShowProperty)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QList<QUrl>);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl() }));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl("file:///") }));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl("file:///"), QUrl("usershare:///hello") }));
}

TEST_F(UT_ShareEventsCaller, SendSwitchDisplayMode)
{
    typedef bool (EventDispatcherManager::*Publish)(int, quint64, int &&);
    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(0, Global::ViewMode::kNoneMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(0, Global::ViewMode::kIconMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kListMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kExtendMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kAllViewMode));
}
