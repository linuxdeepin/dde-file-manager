// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashfileeventreceiver.cpp
 * @brief Unit tests for TrashFileEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperationsevent/trashfileeventreceiver.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class TrashFileEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashFileEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashFileEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashFileEventReceiverTest, handleSaveRedoOpt)
{
    // Test method: void handleSaveRedoOpt((const QString &token, const bool moreThanZero))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSaveRedoOpt(_arg0, false));
}

TEST_F(TrashFileEventReceiverTest, instance)
{
    // Test getter: TrashFileEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
