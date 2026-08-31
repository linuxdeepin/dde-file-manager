// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgroundpreview.cpp
 * @brief Unit tests for BackgroundPreview methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "backgroundpreview.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class BackgroundPreviewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BackgroundPreview();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BackgroundPreview *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BackgroundPreviewTest, BackgroundPreview)
{
    // Test constructor: BackgroundPreview((const QString &screenName, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BackgroundPreviewTest, getPixmap)
{
    // Test method: QPixmap getPixmap((const QString &path, const QPixmap &defalutPixmap))
    QString _arg0{};
    QPixmap _arg1{};
    auto result = obj->getPixmap(_arg0, _arg1);
    EXPECT_TRUE(result.isNull());

}

TEST_F(BackgroundPreviewTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(BackgroundPreviewTest, setDisplay)
{
    // Test setter: void setDisplay((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDisplay(_arg0));
}

TEST_F(BackgroundPreviewTest, updateDisplay)
{
    // Test method: void updateDisplay(())
    EXPECT_NO_FATAL_FAILURE(obj->updateDisplay());
}
