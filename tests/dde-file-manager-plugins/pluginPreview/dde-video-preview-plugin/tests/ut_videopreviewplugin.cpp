/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#include <gtest/gtest.h>

#include "videopreviewplugin.h"

DFM_USE_NAMESPACE

class TestVideoPreviewPlugin :public testing::Test
{
public:
     void SetUp() override
     {
        m_videPreviewPlugin = new VideoPreviewPlugin();
     }

     void TearDown() override
     {
         delete m_videPreviewPlugin;
         m_videPreviewPlugin = nullptr;
     }

public:
     VideoPreviewPlugin * m_videPreviewPlugin;
};

TEST_F(TestVideoPreviewPlugin, use_create)
{
    EXPECT_TRUE(m_videPreviewPlugin->create("dde-file-manager") != nullptr);
}
