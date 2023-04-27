// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "widgets/tagbutton.h"
#include "utils/taghelper.h"

#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagButtonTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&TagHelper::defualtColors, []() {
            __DBG_STUB_INVOKE__
            return QList<QColor>() << QColor("red");
        });
        ins = new TagButton(QColor("red"), nullptr);
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
    TagButton *ins;
};

TEST_F(TagButtonTest, setRadiusF)
{
    ins->setRadiusF(1);
    EXPECT_TRUE(ins->width() == 1);
}

TEST_F(TagButtonTest, setCheckable)
{
    ins->setCheckable(true);
    EXPECT_TRUE(ins->checkable == true);
}

TEST_F(TagButtonTest, isHovered)
{
    EXPECT_FALSE(ins->isHovered());
}

TEST_F(TagButtonTest, enterEvent)
{
    QEvent *event = new QEvent(QEvent::User);
    int flag = 0;
    QObject::connect(ins, &TagButton::enter, [&flag]() {
        flag++;
    });
    ins->enterEvent(event);
    EXPECT_TRUE(flag == 1);
}

TEST_F(TagButtonTest, leaveEvent)
{
    QEvent *event = new QEvent(QEvent::User);
    int flag = 0;
    QObject::connect(ins, &TagButton::leave, [&flag]() {
        flag++;
    });
    ins->leaveEvent(event);
    EXPECT_TRUE(flag == 1);
}

TEST_F(TagButtonTest, mouseEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPoint(100, 100),
                                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    int flag = 0;
    QObject::connect(ins, &TagButton::click, [&flag]() {
        flag++;
    });

    stub.set_lamda(&TagButton::isChecked, []() {
        __DBG_STUB_INVOKE__
        return false;
    });
    ins->mouseReleaseEvent(event);
    ins->mousePressEvent(event);
    EXPECT_TRUE(flag == 1);
    EXPECT_TRUE(ins->paintStatus == TagButton::PaintStatus::kPressed);
}

TEST_F(TagButtonTest, paintEvent)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    bool flag = false;
    auto func = static_cast<void (QPainter::*)(const QRectF &r)>(&QPainter::drawEllipse);
    stub.set_lamda(func, [&flag]() {
        __DBG_STUB_INVOKE__
        flag = true;
    });
    for (auto status : { TagButton::PaintStatus::kNormal,
                         TagButton::PaintStatus::kHover,
                         TagButton::PaintStatus::kPressed,
                         TagButton::PaintStatus::kChecked }) {
        flag = false;
        ins->setPaintStatus(status);
        ins->paintEvent(&event);
        EXPECT_TRUE(flag);
    }
}
