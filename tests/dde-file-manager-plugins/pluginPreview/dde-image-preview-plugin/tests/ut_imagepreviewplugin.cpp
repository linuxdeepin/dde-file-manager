// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "imagepreviewplugin.h"


class TestImagePreviewPlugin : public testing::Test {
public:

    void SetUp() override
    {
        m_imagePreviewPlugin = new ImagePreviewPlugin();
    }

    void TearDown() override
    {
        delete m_imagePreviewPlugin;
        m_imagePreviewPlugin = nullptr;
    }

public:
    ImagePreviewPlugin * m_imagePreviewPlugin;
};

TEST_F(TestImagePreviewPlugin, use_create)
{
    auto *ptr = m_imagePreviewPlugin->create("dde-file-manager");
    EXPECT_TRUE(ptr != nullptr);
    if (ptr) {
        delete ptr;
        ptr = nullptr;
    }
}
