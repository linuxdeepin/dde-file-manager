// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
