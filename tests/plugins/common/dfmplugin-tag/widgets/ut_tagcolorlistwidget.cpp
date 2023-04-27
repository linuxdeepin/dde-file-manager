// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "widgets/tagcolorlistwidget.h"
#include "widgets/tagbutton.h"
#include "utils/taghelper.h"

#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>

#include <QPaintEvent>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagColorListWidgetTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&TagHelper::defualtColors, []() {
            __DBG_STUB_INVOKE__
            return QList<QColor>() << QColor("red");
        });
        ins = new TagColorListWidget();
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (ins) {
            delete ins;
            ins = nullptr;
        }
    }

private:
    stub_ext::StubExt stub;
    TagColorListWidget *ins;
};

TEST_F(TagColorListWidgetTest, checkedColorList)
{
    stub.set_lamda(&TagButton::isChecked, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    EXPECT_FALSE(ins->checkedColorList().isEmpty());
}

TEST_F(TagColorListWidgetTest, exclusive)
{
    EXPECT_FALSE(ins->exclusive());
}

TEST_F(TagColorListWidgetTest, ToolTipText)
{
    stub.set_lamda(&DLabel::isVisible, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&QLabel::setText, [](QLabel *, QString text) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(text == "tag");
    });
    ins->setToolTipText("tag");
    stub.set_lamda(&QLabel::setText, [](QLabel *, QString text) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(text == " ");
    });
    ins->clearToolTipText();
}
