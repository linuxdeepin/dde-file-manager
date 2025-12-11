// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "interface/canvasviewshell.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QMimeData>
#include <QAbstractItemView>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_CanvasViewShell : public testing::Test
{
protected:
    void SetUp() override
    {
        shell = new CanvasViewShell();

        // mock the UI show
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        delete shell;
        shell = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasViewShell *shell = nullptr;
};

TEST_F(UT_CanvasViewShell, Constructor_Default_InitializesCorrectly)
{
    EXPECT_NE(shell, nullptr);
}


TEST_F(UT_CanvasViewShell, initialize_FollowsHooks_ReturnsTrue)
{
    bool result = shell->initialize();
    EXPECT_TRUE(result);
}

TEST_F(UT_CanvasViewShell, gridPos_CallsChannel_ReturnsPoint)
{
    QPoint expectedPoint(3, 4);
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int, const QPoint &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedPoint]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedPoint);
    });

    QPoint result = shell->gridPos(0, QPoint(100, 200));
    EXPECT_EQ(result, expectedPoint);
}

TEST_F(UT_CanvasViewShell, visualRect_CallsChannel_ReturnsRect)
{
    QRect expectedRect(10, 20, 100, 80);
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int, const QUrl &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedRect]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedRect);
    });

    QUrl url("file:///test.txt");
    QRect result = shell->visualRect(0, url);
    EXPECT_EQ(result, expectedRect);
}

TEST_F(UT_CanvasViewShell, gridVisualRect_CallsChannel_ReturnsRect)
{
    QRect expectedRect(50, 60, 80, 80);
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int, const QPoint &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedRect]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedRect);
    });

    QRect result = shell->gridVisualRect(0, QPoint(1, 2));
    EXPECT_EQ(result, expectedRect);
}

TEST_F(UT_CanvasViewShell, gridSize_WithInvalidIndex_ReturnsEmptySize)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), []() {
        __DBG_STUB_INVOKE__
        return QVariant(QSize());
    });

    QSize result = shell->gridSize(-1);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_CanvasViewShell, canvasView_CallsChannel_ReturnsView)
{
    QAbstractItemView *expectedView = nullptr;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedView]() {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(expectedView);
    });

    QAbstractItemView *result = shell->canvasView(0);
    EXPECT_EQ(result, expectedView);
}

TEST_F(UT_CanvasViewShell, eventDropData_SignalNotConnected_ReturnsFalse)
{
    QMimeData mimeData;
    bool result = shell->eventDropData(0, &mimeData, QPoint(0, 0), nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_CanvasViewShell, eventKeyPress_SignalNotConnected_ReturnsFalse)
{
    bool result = shell->eventKeyPress(0, Qt::Key_A, Qt::NoModifier, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_CanvasViewShell, eventShortcutkeyPress_SignalNotConnected_ReturnsFalse)
{
    bool result = shell->eventShortcutkeyPress(0, Qt::Key_A, Qt::ControlModifier, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_CanvasViewShell, eventWheel_SignalNotConnected_ReturnsFalse)
{
    bool result = shell->eventWheel(0, QPoint(0, 120), nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_CanvasViewShell, eventContextMenu_SignalNotConnected_ReturnsFalse)
{
    QUrl dir("file:///home");
    QList<QUrl> files;
    bool result = shell->eventContextMenu(0, dir, files, QPoint(100, 100), nullptr);
    EXPECT_FALSE(result);
}
