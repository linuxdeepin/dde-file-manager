// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "interface/canvasmanagershell.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QSignalSpy>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_CanvasManagerShell : public testing::Test
{
protected:
    void SetUp() override
    {
        shell = new CanvasManagerShell();
    }

    void TearDown() override
    {
        delete shell;
        shell = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasManagerShell *shell = nullptr;
};

TEST_F(UT_CanvasManagerShell, initialize_SubscribesSignals_ReturnsTrue)
{
    bool result = shell->initialize();
    EXPECT_TRUE(result);
}

TEST_F(UT_CanvasManagerShell, iconLevel_CallsChannel_ReturnsLevel)
{
    int expectedLevel = 2;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [expectedLevel]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedLevel);
    });

    int result = shell->iconLevel();
    EXPECT_EQ(result, expectedLevel);
}

TEST_F(UT_CanvasManagerShell, setIconLevel_WithZeroLevel_CallsChannel)
{
    bool channelCalled = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&channelCalled]() {
        __DBG_STUB_INVOKE__
        channelCalled = true;
        return QVariant();
    });

    shell->setIconLevel(0);
    EXPECT_TRUE(channelCalled);
}

