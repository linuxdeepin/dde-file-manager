// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "interface/canvasmodelshell.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_CanvasModelShell : public testing::Test
{
protected:
    void SetUp() override
    {
        shell = new CanvasModelShell();
    }

    void TearDown() override
    {
        delete shell;
        shell = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasModelShell *shell = nullptr;
};

TEST_F(UT_CanvasModelShell, initialize_FollowsHooks_ReturnsTrue)
{
    bool result = shell->initialize();
    EXPECT_TRUE(result);
}

TEST_F(UT_CanvasModelShell, refresh_WithDefaultParams_CallsChannel)
{
    bool channelCalled = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, bool, int &, bool &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&channelCalled]() {
        __DBG_STUB_INVOKE__
        channelCalled = true;
        return QVariant();
    });

    shell->refresh();
    EXPECT_TRUE(channelCalled);
}

TEST_F(UT_CanvasModelShell, fetch_WithValidUrl_ReturnsTrue)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), []() {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    QUrl url("file:///home/test/file.txt");
    bool result = shell->fetch(url);
    EXPECT_TRUE(result);
}

TEST_F(UT_CanvasModelShell, take_WithValidUrl_ReturnsTrue)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), []() {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    QUrl url("file:///home/test/file.txt");
    bool result = shell->take(url);
    EXPECT_TRUE(result);
}

