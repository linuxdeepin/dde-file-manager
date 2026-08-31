// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_emblemhelper_1.cpp
 * @brief Unit tests for EmblemHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/emblemhelper.h"

#include <QTest>

using namespace dfmplugin_emblem;

class EmblemHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EmblemHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EmblemHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EmblemHelperTest, emblemRects)
{
    // Test method: QList<QRectF> emblemRects((const QRectF &paintArea))
    QRectF _arg0{};
    auto result = obj->emblemRects(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EmblemHelperTest, gioEmblemIcons)
{
    // Test method: QList<QIcon> gioEmblemIcons((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->gioEmblemIcons(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EmblemHelperTest, hasEmblem)
{
    // Test bool getter: hasEmblem()
    bool result = obj->hasEmblem();
    EXPECT_FALSE(result);

}

TEST_F(EmblemHelperTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(EmblemHelperTest, isExtEmblemProhibited)
{
    // Test method: bool isExtEmblemProhibited((const FileInfoPointer &info, const QUrl &url))
    FileInfoPointer _arg0{};
    QUrl _arg1{};
    auto result = obj->isExtEmblemProhibited(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(EmblemHelperTest, onEmblemChanged)
{
    // Test method: void onEmblemChanged((const QUrl &url, const Product &product))
    QUrl _arg0{};
    Product _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onEmblemChanged(_arg0, _arg1));
}

TEST_F(EmblemHelperTest, onUrlChanged)
{
    // Test method: bool onUrlChanged((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->onUrlChanged(0, _arg1);
    EXPECT_FALSE(result);

}
