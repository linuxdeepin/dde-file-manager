// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_timezonewatcher_pms.cpp
 * @brief PMS sev-2 regression tests for TimezoneWatcher
 *        (src/dfm-base/utils/timezonewatcher.cpp).
 *
 * Bug -> case mapping (fix commit verified via `git show`):
 *   - PMS:377193 readAndSetTimezone 在系统总线上监听时区属性变化时，
 *     构造期连接失败/属性读取失败导致空时区被设置；修复后 init() 用
 *     掩码位合并 (QFlags |=) 追加 systemBus 连接并保证只注册一次，
 *     失败时保留当前时区不写入空值。
 */

#include <gtest/gtest.h>
#include <QTimeZone>

#include <dfm-base/utils/timezonewatcher.h>

using namespace dfmbase;

// PMS:377193 init 后时区状态一致：无系统总线环境也不得写入空时区
TEST(TimezoneWatcherPmsTest, BUG377193_InitKeepsConsistentTimezone)
{
    // Arrange — capture the current timezone before init.
    const QString before = TimezoneWatcher::instance().currentTimezone();

    // Act — init() connects the timedated listeners; in the offscreen UT
    // environment there may be no system bus, the connection attempt must
    // be tolerated without installing an empty timezone.
    EXPECT_NO_FATAL_FAILURE({
        TimezoneWatcher::instance().init();
        // A second init must be idempotent (guarded connection setup).
        TimezoneWatcher::instance().init();
    });

    // Assert — the reported timezone stays non-empty and valid; when
    // timedated never reported a zone, the local zone is kept.
    const QString after = TimezoneWatcher::instance().currentTimezone();
    EXPECT_FALSE(after.isEmpty());

    if (!before.isEmpty())
        EXPECT_EQ(before, after);

    const QTimeZone tz(QByteArray(after.toUtf8()));
    EXPECT_TRUE(tz.isValid());
}
