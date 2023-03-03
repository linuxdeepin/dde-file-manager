// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizer_defines.h"
#include "options/sizeslider.h"
#include "config/configpresenter.h"
#include "delegate/collectionitemdelegate.h"

#include "stubext.h"

#include <QLabel>

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(SizeSlider, switchMode_view)
{
    SizeSlider obj;
    ASSERT_EQ(obj.slider, nullptr);
    ASSERT_EQ(obj.label, nullptr);

    stub_ext::StubExt stub;
    bool icon = false;
    bool view = false;

    stub.set_lamda(&SizeSlider::resetToIcon, [&icon](){
        icon = true;
    });

    stub.set_lamda(&SizeSlider::resetToView, [&view](){
        view = true;
    });

    obj.switchMode(SizeSlider::Icon);
    EXPECT_NE(obj.slider, nullptr);
    EXPECT_NE(obj.label, nullptr);
    EXPECT_EQ(obj.mode(), SizeSlider::Icon);
    EXPECT_TRUE(icon);
    EXPECT_FALSE(view);

    auto slider = obj.slider;
    auto label = obj.label;
    view = false;
    icon = false;

    obj.switchMode(SizeSlider::View);
    EXPECT_EQ(obj.slider, slider);
    EXPECT_EQ(obj.label, label);
    EXPECT_EQ(obj.mode(), SizeSlider::View);
    EXPECT_TRUE(view);
    EXPECT_FALSE(icon);
}

TEST(SizeSlider, resetToView)
{
    SizeSlider obj;
    obj.slider = new DSlider(Qt::Horizontal, &obj);
    obj.label = new QLabel(&obj);

    obj.resetToView();
    EXPECT_EQ(obj.slider->minimum(), (int)DisplaySize::kSmaller);
    EXPECT_EQ(obj.slider->maximum(), (int)DisplaySize::kLarger);
    EXPECT_EQ(obj.slider->value(), (int)CfgPresenter->displaySize());
}

TEST(SizeSlider, resetToIcon)
{
    SizeSlider obj;
    obj.slider = new DSlider(Qt::Horizontal, &obj);
    obj.label = new QLabel(&obj);

    obj.resetToIcon();
    EXPECT_EQ(obj.slider->minimum(), CollectionItemDelegate::minimumIconLevel());
    EXPECT_EQ(obj.slider->maximum(), CollectionItemDelegate::maximumIconLevel());
}

TEST(SizeSlider, setValue)
{
    SizeSlider obj;
    obj.slider = new DSlider(Qt::Horizontal, &obj);
    obj.slider->slider()->setRange(0, 20);

    obj.setValue(4);
    EXPECT_EQ(obj.slider->value(), 4);
}

TEST(SizeSlider, iconClicked)
{
    SizeSlider obj;
    obj.slider = new DSlider(Qt::Horizontal, &obj);
    obj.slider->slider()->setRange(0, 20);
    obj.slider->slider()->setValue(1);

    obj.iconClicked(DSlider::LeftIcon, true);
    EXPECT_EQ(obj.slider->value(), 0);

    obj.iconClicked(DSlider::RightIcon, true);
    EXPECT_EQ(obj.slider->value(), 1);

    obj.slider->slider()->setValue(0);
    obj.iconClicked(DSlider::LeftIcon, true);
    EXPECT_EQ(obj.slider->value(), 0);

    obj.slider->slider()->setValue(20);
    obj.iconClicked(DSlider::RightIcon, true);
    EXPECT_EQ(obj.slider->value(), 20);
}
