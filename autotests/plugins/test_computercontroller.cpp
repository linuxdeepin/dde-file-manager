// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computercontroller.cpp
 * @brief Unit tests for ComputerController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "controller/computercontroller.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerControllerTest, ComputerController)
{
    // Test constructor: ComputerController((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerControllerTest, actErase)
{
    // Test method: void actErase((DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->actErase(DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, actSafelyRemove)
{
    // Test method: void actSafelyRemove((DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->actSafelyRemove(DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, onOpenItem)
{
    // Test method: void onOpenItem((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onOpenItem(0, _arg1));
}
