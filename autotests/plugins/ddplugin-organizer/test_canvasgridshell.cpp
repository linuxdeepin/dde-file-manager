// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "interface/canvasgridshell.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QPoint>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_CanvasGridShell : public testing::Test
{
protected:
    void SetUp() override
    {
        shell = new CanvasGridShell();
    }

    void TearDown() override
    {
        delete shell;
        shell = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasGridShell *shell = nullptr;
};

TEST_F(UT_CanvasGridShell, Constructor_Default_InitializesCorrectly)
{
    EXPECT_NE(shell, nullptr);
}

TEST_F(UT_CanvasGridShell, Constructor_WithParent_SetsParent)
{
    QObject parent;
    CanvasGridShell shellWithParent(&parent);
    EXPECT_EQ(shellWithParent.parent(), &parent);
}

TEST_F(UT_CanvasGridShell, initialize_Always_ReturnsTrue)
{
    bool result = shell->initialize();
    EXPECT_TRUE(result);
}

TEST_F(UT_CanvasGridShell, item_WithValidParams_ReturnsItemFromChannel)
{
    QString expectedItem = "file:///home/test/file.txt";
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int, const QPoint &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&expectedItem]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedItem);
    });

    QString result = shell->item(0, QPoint(1, 2));
    EXPECT_EQ(result, expectedItem);
}

TEST_F(UT_CanvasGridShell, item_WithInvalidIndex_ReturnsEmptyString)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int &, const QPoint &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), []() {
        __DBG_STUB_INVOKE__
        return QVariant(QString());
    });

    QString result = shell->item(-1, QPoint(0, 0));
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_CanvasGridShell, tryAppendAfter_WithItems_CallsChannel)
{
    bool channelCalled = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QStringList, int &, const QPoint &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&channelCalled]() {
        __DBG_STUB_INVOKE__
        channelCalled = true;
        return QVariant();
    });

    QStringList items = { "item1", "item2" };
    shell->tryAppendAfter(items, 0, QPoint(1, 1));
    EXPECT_TRUE(channelCalled);
}

TEST_F(UT_CanvasGridShell, point_WithValidItem_ReturnsIndex)
{
    int expectedIndex = 1;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString, QPoint *&);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [expectedIndex]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedIndex);
    });

    QPoint pos;
    int result = shell->point("file:///test.txt", &pos);
    EXPECT_EQ(result, expectedIndex);
}

