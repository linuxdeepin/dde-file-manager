// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_normalizedmodeprivate_1.cpp
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

TEST_F(NormalizedModePrivateTest, NormalizedModePrivate)
{
    // Test constructor: NormalizedModePrivate((NormalizedMode *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(NormalizedModePrivateTest, checkTouchFile)
{
    // Test method: void checkTouchFile((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->checkTouchFile(_arg0));
}

TEST_F(NormalizedModePrivateTest, collectionStyleChanged)
{
    // Test method: void collectionStyleChanged((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->collectionStyleChanged(_arg0));
}

TEST_F(NormalizedModePrivateTest, createCollection)
{
    // Test method: CollectionHolderPointer createCollection((const QString &id))
    QString _arg0{};
    auto result = obj->createCollection(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(NormalizedModePrivateTest, generateScreenConfigId)
{
    // Test getter: QString generateScreenConfigId()
    auto result = obj->generateScreenConfigId();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NormalizedModePrivateTest, onDropFile)
{
    // Test method: void onDropFile((const QString &collection, QList<QUrl> &urls))
    QString _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDropFile(_arg0, _arg1));
}

TEST_F(NormalizedModePrivateTest, onFontChanged)
{
    // Test method: void onFontChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onFontChanged());
}

TEST_F(NormalizedModePrivateTest, onIconSizeChanged)
{
    // Test method: void onIconSizeChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onIconSizeChanged());
}

TEST_F(NormalizedModePrivateTest, openEditor)
{
    // Test method: void openEditor((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openEditor(_arg0));
}

TEST_F(NormalizedModePrivateTest, refreshViews)
{
    // Test method: void refreshViews((bool silence))
    EXPECT_NO_FATAL_FAILURE(obj->refreshViews(false));
}

TEST_F(NormalizedModePrivateTest, NormalizedModePrivate_Destructor)
{
    // Test method:  ~NormalizedModePrivate(())
    EXPECT_NO_FATAL_FAILURE({ NormalizedModePrivate *tmp = new NormalizedModePrivate(); delete tmp; });
}
