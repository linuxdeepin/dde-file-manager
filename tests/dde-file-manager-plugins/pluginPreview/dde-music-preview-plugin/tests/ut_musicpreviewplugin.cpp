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

#include "musicpreviewplugin.h"


class TestMusicPreviewPlugin : public testing::Test {
public:

    void SetUp() override
    {
        m_musicPreviewPlugin = new MusicPreviewPlugin();
    }

    void TearDown() override
    {
        delete m_musicPreviewPlugin;
        m_musicPreviewPlugin = nullptr;
    }

public:
    MusicPreviewPlugin * m_musicPreviewPlugin;
};

TEST_F(TestMusicPreviewPlugin, use_create)
{
    auto *ptr = m_musicPreviewPlugin->create("dde-file-manager");
    EXPECT_TRUE(ptr != nullptr);
    if (ptr) {
        delete ptr;
        ptr = nullptr;
    }
}
