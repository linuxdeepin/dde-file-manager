/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <QWidget>
#include <gtest/gtest.h>

#include <DGuiApplicationHelper>

#define protected public

#include "diskpluginitem.h"

DGUI_USE_NAMESPACE

namespace  {
    class TestDiskPluginItem : public testing::Test {
    public:

        void SetUp() override
        {
            mDiskPluginItem.reset( new DiskPluginItem(nullptr));
        }
        void TearDown() override
        {
        }

    public:
        std::shared_ptr<DiskPluginItem> mDiskPluginItem;
    };
}

TEST_F(TestDiskPluginItem, can_set_display_mode)
{
    mDiskPluginItem->setVisible(false);

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    mDiskPluginItem->setDockDisplayMode(Dock::Fashion);

    mDiskPluginItem->paintEvent(nullptr);

    mDiskPluginItem->setVisible(true);
}

TEST_F(TestDiskPluginItem, can_be_updated_from_signals)
{
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    mDiskPluginItem->setDockDisplayMode(Dock::Fashion);
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);
}
