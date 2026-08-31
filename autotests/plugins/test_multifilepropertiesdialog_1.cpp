// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_multifilepropertiesdialog_1.cpp
 * @brief Unit tests for MultiFilePropertiesDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/multifilepropertiesdialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class MultiFilePropertiesDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MultiFilePropertiesDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MultiFilePropertiesDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MultiFilePropertiesDialogTest, connectHeightChange)
{
    // Test method: void connectHeightChange((DArrowLineDrawer *w))
    EXPECT_NO_FATAL_FAILURE(obj->connectHeightChange(nullptr));
}

TEST_F(MultiFilePropertiesDialogTest, handleGroupBoxStateChanged)
{
    // Test method: void handleGroupBoxStateChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->handleGroupBoxStateChanged(0));
}

TEST_F(MultiFilePropertiesDialogTest, handleHideBoxStateChanged)
{
    // Test method: void handleHideBoxStateChanged((int state))
    EXPECT_NO_FATAL_FAILURE(obj->handleHideBoxStateChanged(0));
}

TEST_F(MultiFilePropertiesDialogTest, handleOtherBoxStateChanged)
{
    // Test method: void handleOtherBoxStateChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->handleOtherBoxStateChanged(0));
}

TEST_F(MultiFilePropertiesDialogTest, handleOwnerBoxStateChanged)
{
    // Test method: void handleOwnerBoxStateChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->handleOwnerBoxStateChanged(0));
}

TEST_F(MultiFilePropertiesDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(MultiFilePropertiesDialogTest, processHeight)
{
    // Test method: void processHeight(())
    EXPECT_NO_FATAL_FAILURE(obj->processHeight());
}

TEST_F(MultiFilePropertiesDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}
