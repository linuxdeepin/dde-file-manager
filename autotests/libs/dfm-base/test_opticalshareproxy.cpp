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
    EXPECT_FALSE(proxy.setBurnState(QStringLiteral("sr0"), state));
}

TEST(OpticalShareProxyTest, ClearBurnStateReturnsFalseWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    EXPECT_FALSE(proxy.clearBurnState(QStringLiteral("sr0")));
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
    EXPECT_FALSE(proxy.setBurnAttribute(QStringLiteral("tag1"), attr));
}

TEST(OpticalShareProxyTest, ClearBurnAttributeReturnsFalseWhenServiceUnavailable)
{
    auto &proxy = OpticalShareProxy::instance();
    EXPECT_FALSE(proxy.clearBurnAttribute(QStringLiteral("tag1")));
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
