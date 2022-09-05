// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "textpreviewplugin.h"

DFM_USE_NAMESPACE

class TestTextPreviewPlugin :public testing::Test
{
public:
     void SetUp() override
     {
        m_testPreviewPlugin = new TextPreviewPlugin();
     }

     void TearDown() override
     {
         delete m_testPreviewPlugin;
         m_testPreviewPlugin = nullptr;
     }

public:
     TextPreviewPlugin * m_testPreviewPlugin;
};

TEST_F(TestTextPreviewPlugin, use_create)
{
    auto *ptr = m_testPreviewPlugin->create("dde-file-manager");
    EXPECT_TRUE(ptr != nullptr);
    if (ptr) {
        delete ptr;
        ptr = nullptr;
    }
}
