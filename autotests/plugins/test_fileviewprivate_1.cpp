// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewprivate_1.cpp
 * @brief Unit tests for FileViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/private/fileview_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewPrivateTest, adjustIconModeSpacing)
{
    // Test method: void adjustIconModeSpacing((const QString &strategyName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->adjustIconModeSpacing(_arg0));
}

TEST_F(FileViewPrivateTest, fileViewStateValue)
{
    // Test method: QVariant fileViewStateValue((const QUrl &url, const QString &key, const QVariant &defalutValue))
    QUrl _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    auto result = obj->fileViewStateValue(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewPrivateTest, iconModeColumnCount)
{
    // Test method: int iconModeColumnCount((int itemWidth))
    auto result = obj->iconModeColumnCount(0);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewPrivateTest, initContentLabel)
{
    // Test method: void initContentLabel(())
    EXPECT_NO_FATAL_FAILURE(obj->initContentLabel());
}

TEST_F(FileViewPrivateTest, initIconModeView)
{
    // Test method: void initIconModeView(())
    EXPECT_NO_FATAL_FAILURE(obj->initIconModeView());
}

TEST_F(FileViewPrivateTest, modelIndexUrl)
{
    // Test method: QUrl modelIndexUrl((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->modelIndexUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewPrivateTest, pureResizeEvent)
{
    // Test event handler: pureResizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->pureResizeEvent(&_event));
}

TEST_F(FileViewPrivateTest, selectedDraggableIndexes)
{
    // Test getter: QModelIndexList selectedDraggableIndexes()
    auto result = obj->selectedDraggableIndexes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewPrivateTest, updateHorizontalScrollBarPosition)
{
    // Test method: void updateHorizontalScrollBarPosition(())
    EXPECT_NO_FATAL_FAILURE(obj->updateHorizontalScrollBarPosition());
}
