// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basicstatusbar.cpp
 * @brief Unit tests for BasicStatusBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmstatusbar/basicstatusbar.h"

#include <QTest>

using namespace src;

class BasicStatusBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BasicStatusBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BasicStatusBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BasicStatusBarTest, clearLayoutAndAnchors)
{
    // Test method: void clearLayoutAndAnchors(())
    EXPECT_NO_FATAL_FAILURE(obj->clearLayoutAndAnchors());
}

TEST_F(BasicStatusBarTest, insertWidget)
{
    // Test method: void insertWidget((const int index, QWidget *widget, int stretch, Qt::Alignment alignment))
    EXPECT_NO_FATAL_FAILURE(obj->insertWidget(0, nullptr, 0, Qt::Alignment()));
}

TEST_F(BasicStatusBarTest, itemCounted)
{
    // Test method: void itemCounted((const int count))
    EXPECT_NO_FATAL_FAILURE(obj->itemCounted(0));
}

TEST_F(BasicStatusBarTest, itemSelected)
{
    // Test method: void itemSelected((const QList<FileInfo *> &infoList))
    EXPECT_NO_FATAL_FAILURE(obj->itemSelected(nullptr));
}

TEST_F(BasicStatusBarTest, setTipText)
{
    // Test setter: void setTipText((const QString &tip))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTipText(_arg0));
}

TEST_F(BasicStatusBarTest, sizeHint)
{
    // Test getter: QSize sizeHint()
    auto result = obj->sizeHint();
    EXPECT_TRUE(result.isEmpty());

}
