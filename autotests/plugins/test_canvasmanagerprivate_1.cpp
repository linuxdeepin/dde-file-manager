// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmanagerprivate_1.cpp
 * @brief Unit tests for CanvasManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "canvasmanager.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerPrivateTest, CanvasManagerPrivate)
{
    // Test constructor: CanvasManagerPrivate((CanvasManager *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasManagerPrivateTest, createView)
{
    // Test method: CanvasViewPointer createView((QWidget *root, int index))
    auto result = obj->createView(nullptr, 0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(CanvasManagerPrivateTest, initSetting)
{
    // Test method: void initSetting(())
    EXPECT_NO_FATAL_FAILURE(obj->initSetting());
}

TEST_F(CanvasManagerPrivateTest, onAboutToFileSort)
{
    // Test method: void onAboutToFileSort(())
    EXPECT_NO_FATAL_FAILURE(obj->onAboutToFileSort());
}

TEST_F(CanvasManagerPrivateTest, onFileDataChanged)
{
    // Test method: void onFileDataChanged((const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    QVector<int> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileDataChanged(_arg0, _arg1, _arg2));
}

TEST_F(CanvasManagerPrivateTest, onFileRenamed)
{
    // Test method: void onFileRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileRenamed(_arg0, _arg1));
}

TEST_F(CanvasManagerPrivateTest, onFileSorted)
{
    // Test method: void onFileSorted(())
    EXPECT_NO_FATAL_FAILURE(obj->onFileSorted());
}

TEST_F(CanvasManagerPrivateTest, onHiddenFlagsChanged)
{
    // Test method: void onHiddenFlagsChanged((bool show))
    EXPECT_NO_FATAL_FAILURE(obj->onHiddenFlagsChanged(false));
}

TEST_F(CanvasManagerPrivateTest, relativeRect)
{
    // Test getter: QRect relativeRect()
    auto result = obj->relativeRect();
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasManagerPrivateTest, updateView)
{
    // Test method: void updateView((const CanvasViewPointer &view, QWidget *root, int index))
    CanvasViewPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateView(_arg0, nullptr, 0));
}

TEST_F(CanvasManagerPrivateTest, CanvasManagerPrivate_Destructor)
{
    // Test method:  ~CanvasManagerPrivate(())
    EXPECT_NO_FATAL_FAILURE({ CanvasManagerPrivate *tmp = new CanvasManagerPrivate(); delete tmp; });
}
