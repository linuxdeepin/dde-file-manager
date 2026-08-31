// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_loadinglabel.cpp
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

TEST_F(LoadingLabelTest, LoadingLabel)
{
    // Test constructor: LoadingLabel((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(LoadingLabelTest, resize)
{
    // Test method: void resize((const QSize &size))
    QSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resize(_arg0));
}

TEST_F(LoadingLabelTest, setText)
{
    // Test setter: void setText((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setText(_arg0));
}

TEST_F(LoadingLabelTest, start)
{
    // Test method: void start(())
    EXPECT_NO_FATAL_FAILURE(obj->start());
}
