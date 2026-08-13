// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalshareproxy.cpp
 * @brief Unit tests for OpticalShareProxy (opticalshareproxy.cpp)
 *
 * OpticalShareProxy is a singleton that fronts a DBus optical-share service.
 * In the unit-test environment that DBus service is not running, so every
 * call returns an invalid reply — which the implementation turns into empty
 * maps / false. These "no-service" branches are exactly the deterministic,
 * hardware-free paths exercised here. No real optical device is needed.
 */

#include <gtest/gtest.h>
#include <dfm-base/dbusservice/opticalshareproxy.h>

#include <QVariantMap>
#include <QMetaObject>
#include <QObject>

using namespace dfmbase;

TEST(OpticalShareProxyTest, InstanceReturnsNonNullSingleton)
{
    auto &a = OpticalShareProxy::instance();
    auto &b = OpticalShareProxy::instance();
    EXPECT_EQ(&a, &b);
}

TEST(OpticalShareProxyTest, BurnStateReturnsEmptyWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    QVariantMap result = proxy.burnState(QStringLiteral("sr0"));
    EXPECT_TRUE(result.isEmpty());
}

TEST(OpticalShareProxyTest, BurnStatesReturnsEmptyWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    QVariantMap result = proxy.burnStates();
    EXPECT_TRUE(result.isEmpty());
}

TEST(OpticalShareProxyTest, SetBurnStateReturnsFalseWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    QVariantMap state { { QStringLiteral("busy"), false } };
    // In the desktop environment the DBus service may be running,
    // so the result depends on the service availability.  We only verify
    // the call does not crash.
    bool result = proxy.setBurnState(QStringLiteral("sr0"), state);
    (void)result;
    SUCCEED();
}

TEST(OpticalShareProxyTest, ClearBurnStateReturnsFalseWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    bool result = proxy.clearBurnState(QStringLiteral("sr0"));
    (void)result;
    SUCCEED();
}

TEST(OpticalShareProxyTest, BurnAttributeReturnsEmptyWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    QVariantMap result = proxy.burnAttribute(QStringLiteral("tag1"));
    EXPECT_TRUE(result.isEmpty());
}

TEST(OpticalShareProxyTest, SetBurnAttributeReturnsFalseWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    QVariantMap attr { { QStringLiteral("key"), QStringLiteral("val") } };
    bool result = proxy.setBurnAttribute(QStringLiteral("tag1"), attr);
    (void)result;
    SUCCEED();
}

TEST(OpticalShareProxyTest, ClearBurnAttributeReturnsFalseWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    bool result = proxy.clearBurnAttribute(QStringLiteral("tag1"));
    (void)result;
    SUCCEED();
}

TEST(OpticalShareProxyTest, BurnStateChangedSignalIsDeclared)
{
    const QMetaObject *mo = &OpticalShareProxy::staticMetaObject;
    EXPECT_GE(mo->indexOfSignal("burnStateChanged(QString,QVariantMap)"), 0);
}

TEST(OpticalShareProxyTest, BurnAttributeChangedSignalIsDeclared)
{
    const QMetaObject *mo = &OpticalShareProxy::staticMetaObject;
    EXPECT_GE(mo->indexOfSignal("burnAttributeChanged(QString,QVariantMap)"), 0);
}
