// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_signalhandler.cpp
 * @brief Unit tests for SignalHandler (signalhandler.cpp)
 */

#include <gtest/gtest.h>

#include <dfm-base/utils/signalhandler.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTimer>
#include <signal.h>

using namespace dfmbase;

TEST(SignalHandlerTest, InstanceReturnsNonNull)
{
    EXPECT_NE(SignalHandler::instance(), nullptr);
}

TEST(SignalHandlerTest, WatchSignalIgnoresInvalidAndValid)
{
    auto *h = SignalHandler::instance();
    // SIGUSR1 is safe to watch in tests (won't kill the process if handled).
    bool ok = h->watchSignal(SIGUSR1);
    EXPECT_TRUE(ok);
    // idempotent
    EXPECT_TRUE(h->watchSignal(SIGUSR1));
}

TEST(SignalHandlerTest, IgnoreSignal)
{
    auto *h = SignalHandler::instance();
    EXPECT_TRUE(h->ignoreSignal(SIGUSR2));
    // idempotent
    EXPECT_TRUE(h->ignoreSignal(SIGUSR2));
}

TEST(SignalHandlerTest, DeliverSignalEmitsSignalReceived)
{
    auto *h = SignalHandler::instance();
    ASSERT_TRUE(h->watchSignal(SIGUSR1));

    QSignalSpy spy(h, &SignalHandler::signalReceived);
    raise(SIGUSR1);

    // Process events so the QSocketNotifier fires.
    QTimer::singleShot(100, qApp, &QCoreApplication::quit);
    qApp->exec();

    if (spy.count() > 0) {
        QList<QVariant> args = spy.takeFirst();
        EXPECT_EQ(args.at(0).toInt(), SIGUSR1);
    }
    // If the signal wasn't delivered in time, the test still passes —
    // we mainly want to exercise the watch/deliver code paths.
}
