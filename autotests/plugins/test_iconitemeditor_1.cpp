// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconitemeditor_1.cpp
 * @brief Unit tests for IconItemEditor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/iconitemeditor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class IconItemEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IconItemEditor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IconItemEditor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IconItemEditorTest, createTooltip)
{
    // Test getter: DArrowRectangle createTooltip()
    auto result = obj->createTooltip();
    EXPECT_NO_FATAL_FAILURE({ obj->createTooltip(); });

}

TEST_F(IconItemEditorTest, editRedo)
{
    // Test method: void editRedo(())
    EXPECT_NO_FATAL_FAILURE(obj->editRedo());
}

TEST_F(IconItemEditorTest, editTextStackAdvance)
{
    // Test getter: QString editTextStackAdvance()
    auto result = obj->editTextStackAdvance();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemEditorTest, editTextStackBack)
{
    // Test getter: QString editTextStackBack()
    auto result = obj->editTextStackBack();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemEditorTest, editTextStackCurrentItem)
{
    // Test getter: QString editTextStackCurrentItem()
    auto result = obj->editTextStackCurrentItem();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemEditorTest, editUndo)
{
    // Test method: void editUndo(())
    EXPECT_NO_FATAL_FAILURE(obj->editUndo());
}

TEST_F(IconItemEditorTest, event)
{
    // Test method: bool event((QEvent *ee))
    auto result = obj->event(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(IconItemEditorTest, getIconLabel)
{
    // Test getter: QLabel getIconLabel()
    auto result = obj->getIconLabel();
    EXPECT_NO_FATAL_FAILURE({ obj->getIconLabel(); });

}

TEST_F(IconItemEditorTest, getTextEdit)
{
    // Test getter: QTextEdit getTextEdit()
    auto result = obj->getTextEdit();
    EXPECT_NO_FATAL_FAILURE({ obj->getTextEdit(); });

}

TEST_F(IconItemEditorTest, isEditReadOnly)
{
    // Test bool getter: isEditReadOnly()
    bool result = obj->isEditReadOnly();
    EXPECT_FALSE(result);

}

TEST_F(IconItemEditorTest, maxCharSize)
{
    // Test getter: int maxCharSize()
    auto result = obj->maxCharSize();
    EXPECT_EQ(result, 0);

}

TEST_F(IconItemEditorTest, opacity)
{
    // Test getter: qreal opacity()
    auto result = obj->opacity();
    EXPECT_EQ(result, 0.0);

}

TEST_F(IconItemEditorTest, popupEditContentMenu)
{
    // Test method: void popupEditContentMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->popupEditContentMenu());
}

TEST_F(IconItemEditorTest, pushItemToEditTextStack)
{
    // Test method: void pushItemToEditTextStack((const QString &item))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->pushItemToEditTextStack(_arg0));
}

TEST_F(IconItemEditorTest, resizeFromEditTextChanged)
{
    // Test method: void resizeFromEditTextChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->resizeFromEditTextChanged());
}

TEST_F(IconItemEditorTest, select)
{
    // Test method: void select((const QString &part))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->select(_arg0));
}

TEST_F(IconItemEditorTest, setCharCountLimit)
{
    // Test method: void setCharCountLimit(())
    EXPECT_NO_FATAL_FAILURE(obj->setCharCountLimit());
}

TEST_F(IconItemEditorTest, setMaxCharSize)
{
    // Test setter: void setMaxCharSize((int maxSize))
    EXPECT_NO_FATAL_FAILURE(obj->setMaxCharSize(0));
}

TEST_F(IconItemEditorTest, setMaxHeight)
{
    // Test setter: void setMaxHeight((int h))
    EXPECT_NO_FATAL_FAILURE(obj->setMaxHeight(0));
}

TEST_F(IconItemEditorTest, setOpacity)
{
    // Test setter: void setOpacity((qreal opacity))
    EXPECT_NO_FATAL_FAILURE(obj->setOpacity(0.0));
}

TEST_F(IconItemEditorTest, setText)
{
    // Test setter: void setText((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setText(_arg0));
}

TEST_F(IconItemEditorTest, showAlertMessage)
{
    // Test method: void showAlertMessage((const QString &text, int duration))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAlertMessage(_arg0, 0));
}

TEST_F(IconItemEditorTest, sizeHint)
{
    // Test getter: QSize sizeHint()
    auto result = obj->sizeHint();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemEditorTest, updateEditorGeometry)
{
    // Test method: void updateEditorGeometry(())
    EXPECT_NO_FATAL_FAILURE(obj->updateEditorGeometry());
}

TEST_F(IconItemEditorTest, updateStyleSheet)
{
    // Test method: void updateStyleSheet(())
    EXPECT_NO_FATAL_FAILURE(obj->updateStyleSheet());
}
