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
