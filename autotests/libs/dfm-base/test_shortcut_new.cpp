// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shortcut_new.cpp
 * @brief Unit tests for Shortcut (shortcut/shortcut.cpp) — ctor + toStr.
 *        (The old tests live under autotests/old/; this new file adds coverage
 *        in the active test-dfm-base target.)
 */

#include <gtest/gtest.h>
#include <QString>
#include <QJsonDocument>

#include <dfm-base/shortcut/shortcut.h>

using namespace dfmbase;

TEST(ShortcutNewTest, ConstructAndToStr)
{
    Shortcut s;
    QString result = s.toStr();
    EXPECT_FALSE(result.isEmpty());
}

TEST(ShortcutNewTest, ToStrReturnsValidJson)
{
    Shortcut s;
    QString result = s.toStr();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    EXPECT_TRUE(doc.isObject());
}
