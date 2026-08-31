// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_normalizedmode_1.cpp
 * @brief Unit tests for NormalizedMode methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/normalizedmode.h"

#include <QTest>

using namespace ddplugin_organizer;

class NormalizedModeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NormalizedMode();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NormalizedMode *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NormalizedModeTest, changeCollectionSurface)
{
    // Test method: void changeCollectionSurface((const QString &screenName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->changeCollectionSurface(_arg0));
}

TEST_F(NormalizedModeTest, deactiveAllPredictors)
{
    // Test method: void deactiveAllPredictors(())
    EXPECT_NO_FATAL_FAILURE(obj->deactiveAllPredictors());
}

TEST_F(NormalizedModeTest, detachLayout)
{
    // Test method: void detachLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->detachLayout());
}

TEST_F(NormalizedModeTest, filterContextMenu)
{
    // Test method: bool filterContextMenu((int, const QUrl &, const QList<QUrl> &, const QPoint &))
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    QPoint _arg3{};
    auto result = obj->filterContextMenu(0, _arg1, _arg2, _arg3);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeTest, filterDataInserted)
{
    // Test method: bool filterDataInserted((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->filterDataInserted(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeTest, filterDropData)
{
    // Test method: bool filterDropData((int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData))
    QPoint _arg2{};
    auto result = obj->filterDropData(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeTest, filterKeyPress)
{
    // Test method: bool filterKeyPress((int viewIndex, int key, int modifiers))
    auto result = obj->filterKeyPress(0, 0, 0);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeTest, filterShortcutkeyPress)
{
    // Test method: bool filterShortcutkeyPress((int viewIndex, int key, int modifiers))
    auto result = obj->filterShortcutkeyPress(0, 0, 0);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeTest, initialize)
{
    // Test method: bool initialize((CollectionModel *m))
    auto result = obj->initialize(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeTest, onCollectionEditStatusChanged)
{
    // Test method: void onCollectionEditStatusChanged((bool editing))
    EXPECT_NO_FATAL_FAILURE(obj->onCollectionEditStatusChanged(false));
}

TEST_F(NormalizedModeTest, onCollectionMoving)
{
    // Test method: void onCollectionMoving((bool moving))
    EXPECT_NO_FATAL_FAILURE(obj->onCollectionMoving(false));
}

TEST_F(NormalizedModeTest, onFileDataChanged)
{
    // Test method: void onFileDataChanged((const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    QVector<int> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileDataChanged(_arg0, _arg1, _arg2));
}

TEST_F(NormalizedModeTest, onReorganizeDesktop)
{
    // Test method: void onReorganizeDesktop(())
    EXPECT_NO_FATAL_FAILURE(obj->onReorganizeDesktop());
}

TEST_F(NormalizedModeTest, setClassifier)
{
    // Test method: bool setClassifier((Classifier id))
    auto result = obj->setClassifier(Classifier());
    EXPECT_FALSE(result);

}
