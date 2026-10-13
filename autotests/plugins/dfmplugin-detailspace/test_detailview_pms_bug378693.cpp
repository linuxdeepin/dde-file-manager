// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QByteArray>
#include <QBuffer>
#include <QMovie>

#include "stubext.h"

#include "views/imagepreviewwidget.h"

using namespace dfmplugin_detailspace;

class TestDetailViewBug378693 : public testing::Test
{
protected:
    void SetUp() override
    {
        widget = new ImagePreviewWidget();
    }

    void TearDown() override
    {
        stub.clear();
        delete widget;
        widget = nullptr;
    }

    ImagePreviewWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

// PMS:378693 FTP 远程文件 GIF 动图预览：通过 QBuffer 替代 setFileName
// 修复 (08f44b20): setAnimatedImage 在 data 非空时使用 QBuffer 作为 QMovie 设备，
// stopAnimatedImage 清理 buffer 和 data
TEST_F(TestDetailViewBug378693, SetAnimatedImage_WithData_CreatesBuffer)
{
    // Minimal GIF header (GIF89a) to simulate animated image data
    QByteArray gifData = QByteArray::fromHex("474946383961");
    gifData += QByteArray(10, '\0');

    widget->setAnimatedImage("/tmp/test.gif", gifData);

    // After setAnimatedImage with data, m_buffer should be created
    EXPECT_NE(widget->m_buffer, nullptr);
}

TEST_F(TestDetailViewBug378693, SetAnimatedImage_WithoutData_NoBuffer)
{
    // Without data (empty), should use file path directly, no buffer
    widget->setAnimatedImage("/tmp/test.gif", QByteArray());

    EXPECT_EQ(widget->m_buffer, nullptr);
}

TEST_F(TestDetailViewBug378693, StopAnimatedImage_ClearsBuffer)
{
    // First set animated image with data to create buffer
    QByteArray gifData = QByteArray::fromHex("474946383961");
    gifData += QByteArray(10, '\0');

    widget->setAnimatedImage("/tmp/test.gif", gifData);
    ASSERT_NE(widget->m_buffer, nullptr);

    // Stop should clean up buffer and data
    widget->stopAnimatedImage();

    EXPECT_EQ(widget->m_buffer, nullptr);
    EXPECT_TRUE(widget->m_imageData.isEmpty());
}

TEST_F(TestDetailViewBug378693, StopAnimatedImage_WithoutBuffer_NoCrash)
{
    // Stop without ever setting an animated image should not crash
    EXPECT_NO_FATAL_FAILURE(widget->stopAnimatedImage());
    EXPECT_EQ(widget->m_buffer, nullptr);
}
