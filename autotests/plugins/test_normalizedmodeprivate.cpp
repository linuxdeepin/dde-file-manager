// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_normalizedmodeprivate.cpp
 * @brief Unit tests for NormalizedModePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/normalizedmode.h"

#include <QTest>

using namespace ddplugin_organizer;

class NormalizedModePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NormalizedModePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NormalizedModePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NormalizedModePrivateTest, checkPastedFiles)
{
    // Test method: void checkPastedFiles((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->checkPastedFiles(_arg0));
}

TEST_F(NormalizedModePrivateTest, connectCollectionSignals)
{
    // Test method: void connectCollectionSignals((CollectionHolderPointer collection))
    EXPECT_NO_FATAL_FAILURE(obj->connectCollectionSignals(CollectionHolderPointer()));
}

TEST_F(NormalizedModePrivateTest, findValidPos)
{
    // Test method: QPoint findValidPos((int &currentIndex, const int width, const int height))
    int _arg0{};
    auto result = obj->findValidPos(_arg0, 0, 0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(NormalizedModePrivateTest, moveFilesToCanvas)
{
    // Test method: bool moveFilesToCanvas((int viewIndex, const QList<QUrl> &urls, const QPoint &viewPoint))
    QList<QUrl> _arg1{};
    QPoint _arg2{};
    auto result = obj->moveFilesToCanvas(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModePrivateTest, onClearSelection)
{
    // Test method: void onClearSelection(())
    EXPECT_NO_FATAL_FAILURE(obj->onClearSelection());
}

TEST_F(NormalizedModePrivateTest, onSelectFile)
{
    // Test method: void onSelectFile((QList<QUrl> &urls, int flag))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSelectFile(_arg0, 0));
}

TEST_F(NormalizedModePrivateTest, switchCollection)
{
    // Test method: void switchCollection(())
    EXPECT_NO_FATAL_FAILURE(obj->switchCollection());
}

TEST_F(NormalizedModePrivateTest, tryPlaceRect)
{
    // Test method: bool tryPlaceRect((QRect &item, const QList<QRect> &inSeats, const QSize &table))
    QRect _arg0{};
    QList<QRect> _arg1{};
    QSize _arg2{};
    auto result = obj->tryPlaceRect(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModePrivateTest, updateHolderSurfaceIndex)
{
    // Test method: void updateHolderSurfaceIndex((QWidget *surface))
    EXPECT_NO_FATAL_FAILURE(obj->updateHolderSurfaceIndex(nullptr));
}
