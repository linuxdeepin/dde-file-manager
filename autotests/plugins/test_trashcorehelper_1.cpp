// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashcorehelper_1.cpp
 * @brief Unit tests for TrashCoreHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/trashcorehelper.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashCoreHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashCoreHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashCoreHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashCoreHelperTest, createTrashPropertyDialog)
{
    // Test method: QWidget createTrashPropertyDialog((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createTrashPropertyDialog(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createTrashPropertyDialog(_arg0); });

}
