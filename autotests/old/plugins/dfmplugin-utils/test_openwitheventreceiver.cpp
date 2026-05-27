// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/openwith/openwitheventreceiver.h"
#include "plugins/common/dfmplugin-utils/openwith/openwithdialog.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/dpf.h>

#include <QApplication>
#include <QWidget>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_OpenWithEventReceiver : public testing::Test
{
protected:
    void SetUp() override
    {
        receiver = new OpenWithEventReceiver();
    }

    void TearDown() override
    {
        delete receiver;
        receiver = nullptr;
        stub.clear();
    }

    OpenWithEventReceiver *receiver { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_OpenWithEventReceiver, initEventConnect_ConnectsSlot)
{
    bool connected = false;

    typedef bool (EventChannelManager::*Connect)(const QString &, const QString &, OpenWithEventReceiver *, decltype(&OpenWithEventReceiver::showOpenWithDialog));
    stub.set_lamda(static_cast<Connect>(&EventChannelManager::connect),
                   [&connected] {
                       __DBG_STUB_INVOKE__
                       connected = true;
                       return true;
                   });

    receiver->initEventConnect();

    EXPECT_TRUE(connected);
}

TEST_F(UT_OpenWithEventReceiver, showOpenWithDialog_WithValidWinId_FindsWindow)
{
    quint64 winId = 12345;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };

    FileManagerWindow mockWindow(QUrl::fromLocalFile("/home"));
    stub.set_lamda(ADDR(FileManagerWindowsManager, findWindowById),
                   [&mockWindow] {
                       __DBG_STUB_INVOKE__
                       return &mockWindow;
                   });

    stub.set_lamda(VADDR(OpenWithDialog, exec),
                   [] {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    receiver->showOpenWithDialog(winId, urls);
}

TEST_F(UT_OpenWithEventReceiver, showOpenWithDialog_WithZeroWinId_NoParent)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };

    stub.set_lamda(VADDR(OpenWithDialog, exec),
                   [] {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    receiver->showOpenWithDialog(0, urls);
}
