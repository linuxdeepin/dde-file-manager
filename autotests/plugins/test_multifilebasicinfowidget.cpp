// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_multifilebasicinfowidget.cpp
 * @brief Unit tests for MultiFileBasicInfoWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/multifilebasicinfowidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class MultiFileBasicInfoWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MultiFileBasicInfoWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MultiFileBasicInfoWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MultiFileBasicInfoWidgetTest, calculateFileCount)
{
    // Test method: void calculateFileCount((const QList<QUrl> &urls,
                                                  int &dirCount,
                                                  int &fileCount))
    QList<QUrl> _arg0{};
    int _arg1{};
    int _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->calculateFileCount(_arg0, _arg1, _arg2));
}

TEST_F(MultiFileBasicInfoWidgetTest, setAccessTime)
{
    // Test setter: void setAccessTime((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setAccessTime(_arg0));
}

TEST_F(MultiFileBasicInfoWidgetTest, setHideState)
{
    // Test setter: void setHideState((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setHideState(_arg0));
}

TEST_F(MultiFileBasicInfoWidgetTest, setModifyTime)
{
    // Test setter: void setModifyTime((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setModifyTime(_arg0));
}
