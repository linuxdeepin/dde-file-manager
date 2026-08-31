// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textbrowseredit.cpp
 * @brief Unit tests for TextBrowserEdit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/text-preview/textbrowseredit.h"

#include <QTest>

using namespace src;

class TextBrowserEditTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextBrowserEdit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextBrowserEdit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextBrowserEditTest, verifyEndOfStrIntegrity)
{
    // Test method: int verifyEndOfStrIntegrity((const char *s, int l))
    auto result = obj->verifyEndOfStrIntegrity(nullptr, 0);
    EXPECT_GE(result, 0);

}
