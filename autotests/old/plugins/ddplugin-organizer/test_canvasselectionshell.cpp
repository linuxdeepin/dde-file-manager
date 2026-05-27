// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "interface/canvasselectionshell.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QItemSelectionModel>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_CanvasSelectionShell : public testing::Test
{
protected:
    void SetUp() override
    {
        shell = new CanvasSelectionShell();
    }

    void TearDown() override
    {
        delete shell;
        shell = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasSelectionShell *shell = nullptr;
};

TEST_F(UT_CanvasSelectionShell, Constructor_Default_InitializesCorrectly)
{
    EXPECT_NE(shell, nullptr);
}

TEST_F(UT_CanvasSelectionShell, initialize_SubscribesSignal_ReturnsTrue)
{
    bool result = shell->initialize();
    EXPECT_TRUE(result);
}

TEST_F(UT_CanvasSelectionShell, selectionModel_CallsChannel_ReturnsModel)
{
    QItemSelectionModel *expectedModel = nullptr;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedModel]() {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(expectedModel);
    });

    QItemSelectionModel *result = shell->selectionModel();
    EXPECT_EQ(result, expectedModel);
}


