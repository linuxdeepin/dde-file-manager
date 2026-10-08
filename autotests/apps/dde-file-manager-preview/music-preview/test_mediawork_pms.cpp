// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "mediawork.h"

#include <gtest/gtest.h>

#include <QAudioOutput>
#include <QMediaPlayer>

using namespace plugin_filepreview;

// PMS:321375 音乐文件空格预览无声音输出（Qt6 需显式设置音频输出）
class MediaWorkPMS : public testing::Test
{
protected:
    void SetUp() override
    {
        work = new MediaWork();
    }

    void TearDown() override
    {
        delete work;
        work = nullptr;
        stub.clear();
    }

    MediaWork *work { nullptr };
    stub_ext::StubExt stub;
};

// PMS:321375 修复前：Qt6 下 createMediaPlayer 未创建/挂载 QAudioOutput，
// QMediaPlayer 无音频输出设备，空格预览音乐无声。
TEST_F(MediaWorkPMS, BUG321375_CreateMediaPlayer_AttachesAudioOutputInQt6)
{
    work->createMediaPlayer();

    ASSERT_NE(work->mediaPlayer, nullptr);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    ASSERT_NE(work->audioOutput, nullptr);
    // 音频输出必须真正挂载到播放器上，否则预览播放无声
    EXPECT_EQ(work->mediaPlayer->audioOutput(), work->audioOutput);
#else
    SUCCEED() << "Qt5 path does not require explicit QAudioOutput";
#endif
}

// PMS:321375 修复实现中 QAudioOutput 以 MediaWork 为父对象，随 worker 一起释放，避免泄漏
TEST_F(MediaWorkPMS, BUG321375_AudioOutput_OwnedByMediaWork)
{
    work->createMediaPlayer();

    ASSERT_NE(work->mediaPlayer, nullptr);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    ASSERT_NE(work->audioOutput, nullptr);
    EXPECT_EQ(work->audioOutput->parent(), work);
#else
    SUCCEED() << "Qt5 path does not create QAudioOutput";
#endif
}
