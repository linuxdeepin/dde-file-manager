// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_thumbnailcreators_pms.cpp
 * @brief PMS sev-2 regression tests for ThumbnailCreators::videoThumbnailCreatorFfmpeg
 *        (src/dfm-base/utils/thumbnailcreators.cpp).
 *
 * Bug -> case mapping (fix commit verified via `git show`):
 *   - PMS:210633 ffprobe 输出解析越界：ffprobe 失败/输出为空或非数字
 *     时，旧的解析方式会越界访问 split 结果导致崩溃；修复后对空输出
 *     与非数字时长安全返回 null 图片。
 *
 * The ffmpeg/ffprobe processes are stubbed out (QProcess::start /
 * waitForFinished / readAllStandardOutput) so no real binaries are
 * needed in the UT environment.
 */

#include <gtest/gtest.h>
#include <QProcess>
#include <QImage>

#include "stubext.h"

#include <dfm-base/utils/thumbnail/thumbnailcreators.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;
using namespace ThumbnailCreators;
using DFMGLOBAL_NAMESPACE::ThumbnailSize;

class UT_ThumbnailCreatorsPms : public ::testing::Test
{
protected:
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

// PMS:210633 ffprobe 输出为空/非数字时长时必须安全返回 null 图片，不得越界
TEST_F(UT_ThumbnailCreatorsPms, BUG210633_EmptyOrGarbageFfprobeOutputNoCrash)
{
    // Arrange — ffprobe "runs fine" but prints nothing at all.
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                   [](QProcess *, int) -> bool { return true; });
    stub.set_lamda(&QProcess::readAllStandardOutput,
                   [](QProcess *) -> QByteArray { return QByteArray(); });

    QImage img;
    EXPECT_NO_FATAL_FAILURE({
        img = videoThumbnailCreatorFfmpeg(
                QStringLiteral("/no/such/video_210633.mp4"), ThumbnailSize::kNormal);
    });

    // Assert — empty/garbage duration produces no thumbnail.
    EXPECT_TRUE(img.isNull());

    // Re-run with garbage (non-numeric) output: must not crash either.
    stub.set_lamda(&QProcess::readAllStandardOutput,
                   [](QProcess *) -> QByteArray { return QByteArray("garbage"); });
    EXPECT_NO_FATAL_FAILURE({
        img = videoThumbnailCreatorFfmpeg(
                QStringLiteral("/no/such/video_210633.mp4"), ThumbnailSize::kNormal);
    });
    EXPECT_TRUE(img.isNull());
}

// PMS:210633 ffprobe 超时/启动失败路径同样必须干净地返回 null 图片
TEST_F(UT_ThumbnailCreatorsPms, BUG210633_FfprobeTimeoutReturnsNullImage)
{
    // Arrange — the probe never finishes.
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                   [](QProcess *, int) -> bool { return false; });

    // Act / Assert
    QImage img;
    EXPECT_NO_FATAL_FAILURE({
        img = videoThumbnailCreatorFfmpeg(
                QStringLiteral("/no/such/video_210633.mp4"), ThumbnailSize::kNormal);
    });
    EXPECT_TRUE(img.isNull());
}
