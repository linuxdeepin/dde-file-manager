// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDER_CONTENT_TEST_HELPERS_H
#define FOLDER_CONTENT_TEST_HELPERS_H

#include <gtest/gtest.h>
#include <QDeadlineTimer>
#include <QIcon>
#include <QQmlError>
#include <QSignalSpy>
#include <QTest>

// gtest-friendly replacement for QTest's QTRY_* macros: poll @p cond until it
// holds or @p timeoutMs elapse, processing events in between. Unlike the
// QTest macros, a timeout reports a proper gtest failure via ASSERT_TRUE().
template<typename Cond>
bool waitUntil(Cond &&cond, int timeoutMs = 5000)
{
    QDeadlineTimer timer(timeoutMs);
    while (!cond() && !timer.hasExpired())
        QTest::qWait(50);
    return cond();
}

// Offscreen software rendering cannot load DCI theme icons
// (image://dtk.dci.icon); the engine falls back to QIcon and only logs a
// warning. Those provider warnings are environmental, not plugin defects,
// so count only the remaining QML errors.
inline int qmlErrorCount(const QSignalSpy &warnings)
{
    int count = 0;
    for (const auto &arguments : warnings) {
        const auto errors = arguments.at(0).value<QList<QQmlError>>();
        for (const auto &error : errors) {
            if (!error.description().contains(QLatin1String("Failed to get image from provider")))
                ++count;
        }
    }
    return count;
}

#endif  // FOLDER_CONTENT_TEST_HELPERS_H
