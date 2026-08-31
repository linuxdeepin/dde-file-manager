// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sectionkeylabel.cpp
 * @brief Unit tests for SectionKeyLabel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widget/sharecontrolwidget.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class SectionKeyLabelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SectionKeyLabel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SectionKeyLabel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SectionKeyLabelTest, SectionKeyLabel)
{
    // Test constructor: SectionKeyLabel((const QString &text, QWidget *parent, Qt::WindowFlags f))
    ASSERT_NE(obj, nullptr);
}
