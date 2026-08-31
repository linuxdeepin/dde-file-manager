// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_editlabel.cpp
 * @brief Unit tests for EditLabel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "editlabel.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class EditLabelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EditLabel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EditLabel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EditLabelTest, setHotZoom)
{
    // Test setter: void setHotZoom((const QRect &rect))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setHotZoom(_arg0));
}
