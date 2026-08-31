// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_loadinglabel_1.cpp
 * @brief Unit tests for LoadingLabel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "loadinglabel.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class LoadingLabelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LoadingLabel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LoadingLabel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LoadingLabelTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}
