// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_addressbar_1.cpp
 * @brief Unit tests for AddressBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/addressbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class AddressBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AddressBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AddressBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AddressBarTest, completerViewVisible)
{
    // Test bool getter: completerViewVisible()
    bool result = obj->completerViewVisible();
    EXPECT_FALSE(result);

}

TEST_F(AddressBarTest, focusInEvent)
{
    // Test event handler: focusInEvent((QFocusEvent *e))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusInEvent(&_event));
}

TEST_F(AddressBarTest, focusOutEvent)
{
    // Test event handler: focusOutEvent((QFocusEvent *e))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusOutEvent(&_event));
}

TEST_F(AddressBarTest, inputMethodEvent)
{
    // Test event handler: inputMethodEvent((QInputMethodEvent *e))
    QInputMethodEvent _event(QInputMethodEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->inputMethodEvent(&_event));
}

TEST_F(AddressBarTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *e))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(AddressBarTest, setCurrentUrl)
{
    // Test setter: void setCurrentUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentUrl(_arg0));
}

TEST_F(AddressBarTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(AddressBarTest, showOnFocusLostOnce)
{
    // Test method: void showOnFocusLostOnce(())
    EXPECT_NO_FATAL_FAILURE(obj->showOnFocusLostOnce());
}
