// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
