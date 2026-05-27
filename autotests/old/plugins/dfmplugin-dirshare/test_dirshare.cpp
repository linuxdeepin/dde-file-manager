// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "dirshare.h"
#include "utils/usersharehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QWidget>

using namespace dfmplugin_dirshare;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

using namespace dfmplugin_dirshare;

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

class UT_DirShare : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ins = new DirShare();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete ins;
    }

private:
    stub_ext::StubExt stub;
    DirShare *ins { nullptr };
};

TEST_F(UT_DirShare, Initialize)
{
    stub.set_lamda(&DirShare::bindEvents, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(ins->initialize());
}

TEST_F(UT_DirShare, Start)
{
    stub.set_lamda(&DirShare::bindScene, [] { __DBG_STUB_INVOKE__ });
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, CustomViewExtensionView, const QString &&, const int &&);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_TRUE(ins->start());
}

TEST_F(UT_DirShare, CreateShareControlWidget)
{
    EXPECT_EQ(nullptr, ins->createShareControlWidget(QUrl("smb://1.2.3.4")));

    stub.set_lamda(&UserShareHelper::needDisableShareWidget, [] { __DBG_STUB_INVOKE__ return true; });
    bool canShare = false;
    stub.set_lamda(&UserShareHelper::canShare, [&] { __DBG_STUB_INVOKE__ return canShare; });

    EXPECT_EQ(nullptr, ins->createShareControlWidget(QUrl::fromLocalFile("/home")));
    canShare = true;
    EXPECT_NO_FATAL_FAILURE(QScopedPointer<QWidget>(ins->createShareControlWidget(QUrl::fromLocalFile("/home"))));
}

TEST_F(UT_DirShare, BindScene)
{
    typedef QVariant (dpf::EventChannelManager::*Push1)(const QString &, const QString &, QString);
    auto menu_contains_event_caller = static_cast<Push1>(&dpf::EventChannelManager::push);
    bool contains = true;
    stub.set_lamda(menu_contains_event_caller, [&] { __DBG_STUB_INVOKE__ return contains; });

    typedef QVariant (dpf::EventChannelManager::*Push2)(const QString &, const QString &, QString, const QString &);
    auto menu_bind_event_caller = static_cast<Push1>(&dpf::EventChannelManager::push);
    stub.set_lamda(menu_bind_event_caller, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(ins->bindScene("sss"));
    EXPECT_NO_FATAL_FAILURE(ins->bindScene(""));

    typedef bool (dpf::EventDispatcherManager::*Subscribe)(const QString &, const QString &, DirShare *, decltype(&DirShare::bindSceneOnAdded));
    auto sub = static_cast<Subscribe>(&dpf::EventDispatcherManager::subscribe);
    stub.set_lamda(sub, [] { __DBG_STUB_INVOKE__ return true; });

    contains = false;
    EXPECT_NO_FATAL_FAILURE(ins->bindScene("sss"));
    EXPECT_NO_FATAL_FAILURE(ins->bindScene(""));
}

TEST_F(UT_DirShare, BindSceneOnAdded)
{
    EXPECT_NO_FATAL_FAILURE(ins->waitToBind.insert("Hello"));
    EXPECT_TRUE(ins->waitToBind.count() == 1);

    typedef bool (dpf::EventDispatcherManager::*Unsubscribe)(const QString &, const QString &, DirShare *, decltype(&DirShare::bindSceneOnAdded));
    auto unsub = static_cast<Unsubscribe>(&dpf::EventDispatcherManager::unsubscribe);
    stub.set_lamda(unsub, [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&DirShare::bindScene, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->bindSceneOnAdded("Hello"));
    EXPECT_NO_FATAL_FAILURE(ins->bindSceneOnAdded("World"));
}

TEST_F(UT_DirShare, BindEvents)
{
    EXPECT_NO_FATAL_FAILURE(ins->bindEvents());
}

TEST_F(UT_DirShare, OnShareStateChanged)
{
    EXPECT_NO_FATAL_FAILURE(ins->onShareStateChanged(""));

    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QUrl);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins->onShareStateChanged("/home"));
}
