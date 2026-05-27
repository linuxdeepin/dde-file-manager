// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "broker/collectionhookinterface.h"

#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QPoint>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionViewItem>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_CollectionHookInterface : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionHookInterface, dropData_WithValidParams_CallsHook)
{
    bool hookCalled = false;
    typedef bool (EventSequenceManager::*Run)(const QString &, const QString &, QString, const QMimeData *&, const QPoint &, void *&);
    stub.set_lamda(static_cast<Run>(&EventSequenceManager::run), [&hookCalled]() {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return true;
    });

    QString viewId = "test_view";
    QMimeData mimeData;
    QPoint viewPos(100, 200);

    bool result = CollectionHookInterface::dropData(viewId, &mimeData, viewPos);
    EXPECT_TRUE(hookCalled);
}

TEST_F(UT_CollectionHookInterface, keyPress_WithValidParams_CallsHook)
{
    bool hookCalled = false;
    typedef bool (EventSequenceManager::*Run)(const QString &, const QString &, QString , int &, int &, void *&);
    stub.set_lamda(static_cast<Run>(&EventSequenceManager::run), [&hookCalled]() {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return true;
    });

    QString viewId = "test_view";
    int key = Qt::Key_Enter;
    int modifiers = Qt::NoModifier;

    bool result = CollectionHookInterface::keyPress(viewId, key, modifiers);
    EXPECT_TRUE(hookCalled);
}

TEST_F(UT_CollectionHookInterface, startDrag_WithValidParams_CallsHook)
{
    bool hookCalled = false;
    typedef bool (EventSequenceManager::*Run)(const QString &, const QString &, QString, int &, void *&);
    stub.set_lamda(static_cast<Run>(&EventSequenceManager::run), [&hookCalled]() {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return true;
    });

    QString viewId = "test_view";
    int supportedActions = Qt::CopyAction;

    bool result = CollectionHookInterface::startDrag(viewId, supportedActions);
    EXPECT_TRUE(hookCalled);
}

TEST_F(UT_CollectionHookInterface, dragMove_WithValidParams_CallsHook)
{
    bool hookCalled = false;
    typedef bool (EventSequenceManager::*Run)(const QString &, const QString &, QString ,const QMimeData *&, const QPoint &, void *&);
    stub.set_lamda(static_cast<Run>(&EventSequenceManager::run), [&hookCalled]() {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return true;
    });

    QString viewId = "test_view";
    QMimeData mimeData;
    QPoint viewPos(150, 250);

    bool result = CollectionHookInterface::dragMove(viewId, &mimeData, viewPos);
    EXPECT_TRUE(hookCalled);
}

TEST_F(UT_CollectionHookInterface, keyboardSearch_WithValidParams_CallsHook)
{
    bool hookCalled = false;
    typedef bool (EventSequenceManager::*Run)(const QString &, const QString &, QString , const QString &, void *&);
    stub.set_lamda(static_cast<Run>(&EventSequenceManager::run), [&hookCalled]() {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return true;
    });

    QString viewId = "test_view";
    QString search = "test_search";

    bool result = CollectionHookInterface::keyboardSearch(viewId, search);
    EXPECT_TRUE(hookCalled);
}

TEST_F(UT_CollectionHookInterface, drawFile_WithValidParams_CallsHook)
{
    bool hookCalled = false;
    typedef bool (EventSequenceManager::*Run)(const QString &, const QString &, QString, const QUrl &,
                                              QPainter *&, const QStyleOptionViewItem *&, void *&);
    stub.set_lamda(static_cast<Run>(&EventSequenceManager::run), [&hookCalled]() {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return true;
    });

    QString viewId = "test_view";
    QUrl file("file:///tmp/testfile.txt");
    QPainter *painter = nullptr;
    QStyleOptionViewItem *option = nullptr;

    bool result = CollectionHookInterface::drawFile(viewId, file, painter, option);
    EXPECT_TRUE(hookCalled);
}

TEST_F(UT_CollectionHookInterface, shortcutkeyPress_WithValidParams_CallsHook)
{
    bool hookCalled = false;
    typedef bool (EventSequenceManager::*Run)(const QString &, const QString &, QString, int &, int &, void *&);
    stub.set_lamda(static_cast<Run>(&EventSequenceManager::run), [&hookCalled]() {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return true;
    });

    QString viewId = "test_view";
    int key = Qt::Key_F5;
    int modifiers = Qt::NoModifier;

    bool result = CollectionHookInterface::shortcutkeyPress(viewId, key, modifiers);
    EXPECT_TRUE(hookCalled);
}
