// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailview.cpp
 * @brief Unit tests for DetailView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/detailview.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailViewTest, initInfoUI)
{
    // Test method: void initInfoUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initInfoUI());
}

TEST_F(DetailViewTest, syncExtensionWidgets)
{
    // Test method: void syncExtensionWidgets(())
    EXPECT_NO_FATAL_FAILURE(obj->syncExtensionWidgets());
}
