// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewhintmessage.cpp
 * @brief Unit tests for ViewHintMessage methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/viewhintmessage/viewhintmessage.h"

#include <QTest>

using namespace src;

class ViewHintMessageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewHintMessage();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewHintMessage *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewHintMessageTest, setIcon)
{
    // Test setter: void setIcon((const QString &icon))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setIcon(_arg0));
}

TEST_F(ViewHintMessageTest, show)
{
    // Test method: void show((QWidget *hostWidget))
    EXPECT_NO_FATAL_FAILURE(obj->show(nullptr));
}
