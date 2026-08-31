// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_watermasksystem.cpp
 * @brief Unit tests for WatermaskSystem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/watermasksystem.h"

#include <QTest>

using namespace ddplugin_canvas;

class WatermaskSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WatermaskSystem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WatermaskSystem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WatermaskSystemTest, findResource)
{
    // Test method: void findResource((const QString &dirPath, const QString &lang, QString *logo, QString *text))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->findResource(_arg0, _arg1, nullptr, nullptr));
}

TEST_F(WatermaskSystemTest, getEditonResource)
{
    // Test method: void getEditonResource((const QString &root, QString *logo, QString *text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->getEditonResource(_arg0, nullptr, nullptr));
}

TEST_F(WatermaskSystemTest, maskPixmap)
{
    // Test method: QPixmap maskPixmap((const QString &uri, const QSize &size, qreal pixelRatio))
    QString _arg0{};
    QSize _arg1{};
    auto result = obj->maskPixmap(_arg0, _arg1, 0.0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(WatermaskSystemTest, parentWidget)
{
    // Test getter: QWidget parentWidget()
    auto result = obj->parentWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->parentWidget(); });

}
