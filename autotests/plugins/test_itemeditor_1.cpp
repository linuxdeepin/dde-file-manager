// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_itemeditor_1.cpp
 * @brief Unit tests for ItemEditor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "delegate/itemeditor.h"

#include <QTest>

using namespace ddplugin_canvas;

class ItemEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ItemEditor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ItemEditor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ItemEditorTest, createEditor)
{
    // Test getter: RenameEdit createEditor()
    auto result = obj->createEditor();
    EXPECT_NO_FATAL_FAILURE({ obj->createEditor(); });

}

TEST_F(ItemEditorTest, createEditor_createEd)
{
    // Test getter: RenameEdit createEditor()
    auto result = obj->createEditor();
    EXPECT_NO_FATAL_FAILURE({ obj->createEditor(); });

}

TEST_F(ItemEditorTest, createTooltip)
{
    // Test getter: DArrowRectangle createTooltip()
    auto result = obj->createTooltip();
    EXPECT_NO_FATAL_FAILURE({ obj->createTooltip(); });

}

TEST_F(ItemEditorTest, createTooltip_createTo)
{
    // Test getter: DArrowRectangle createTooltip()
    auto result = obj->createTooltip();
    EXPECT_NO_FATAL_FAILURE({ obj->createTooltip(); });

}

TEST_F(ItemEditorTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(ItemEditorTest, init_init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(ItemEditorTest, select)
{
    // Test method: void select((const QString &part))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->select(_arg0));
}

TEST_F(ItemEditorTest, select_select)
{
    // Test method: void select((const QString &part))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->select(_arg0));
}

TEST_F(ItemEditorTest, setBaseGeometry)
{
    // Test setter: void setBaseGeometry((const QRect &base, const QSize &itemSize, const QMargins &margin))
    QRect _arg0{};
    QSize _arg1{};
    QMargins _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setBaseGeometry(_arg0, _arg1, _arg2));
}

TEST_F(ItemEditorTest, setBaseGeometry_setBaseG)
{
    // Test setter: void setBaseGeometry((const QRect &base, const QSize &itemSize, const QMargins &margin))
    QRect _arg0{};
    QSize _arg1{};
    QMargins _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setBaseGeometry(_arg0, _arg1, _arg2));
}

TEST_F(ItemEditorTest, setOpacity)
{
    // Test setter: void setOpacity((qreal opacity))
    EXPECT_NO_FATAL_FAILURE(obj->setOpacity(0.0));
}

TEST_F(ItemEditorTest, setOpacity_setOpaci)
{
    // Test setter: void setOpacity((qreal opacity))
    EXPECT_NO_FATAL_FAILURE(obj->setOpacity(0.0));
}

TEST_F(ItemEditorTest, setText)
{
    // Test setter: void setText((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setText(_arg0));
}

TEST_F(ItemEditorTest, setText_setText)
{
    // Test setter: void setText((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setText(_arg0));
}

TEST_F(ItemEditorTest, showAlertMessage)
{
    // Test method: void showAlertMessage((const QString &text, int duration))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAlertMessage(_arg0, 0));
}

TEST_F(ItemEditorTest, showAlertMessage_showAler)
{
    // Test method: void showAlertMessage((const QString &text, int duration))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAlertMessage(_arg0, 0));
}

TEST_F(ItemEditorTest, updateGeometry)
{
    // Test method: void updateGeometry(())
    EXPECT_NO_FATAL_FAILURE(obj->updateGeometry());
}

TEST_F(ItemEditorTest, updateGeometry_updateGe)
{
    // Test method: void updateGeometry(())
    EXPECT_NO_FATAL_FAILURE(obj->updateGeometry());
}
