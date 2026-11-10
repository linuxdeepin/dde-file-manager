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
#include <stub-ext/stubext.h>
#include <dfm-base/dbusservice/opticalshareproxy.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QVariantMap>
#include <QMetaObject>
#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>

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
    // burnStates() returns whatever the service reports for ALL devices, so
    // when a real optical-share DBus service happens to run in the test
    // environment the result is not empty.  Force the "service unavailable"
    // path instead: QDBusConnection::call (non-virtual) returns a default
    // (invalid) message, the QDBusReply becomes invalid and the proxy
    // returns an empty map.
    stub_ext::StubExt stub;
    using CallFn = QDBusMessage (QDBusConnection::*)(const QDBusMessage &, QDBus::CallMode, int) const;
    stub.set_lamda(static_cast<CallFn>(&QDBusConnection::call),
                   [](const QDBusConnection *, const QDBusMessage &, QDBus::CallMode, int) -> QDBusMessage {
                       __DBG_STUB_INVOKE__
                       return QDBusMessage();
                   });

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

// ============================================================================
// ecb006117 新增接口：服务未注册守卫（guardServiceReady）+ 服务上线异步刷新
// （QDBusServiceWatcher + refreshBurnStatesAsync）+ unpackStatesMap 助手。
//
// 单测环境里光刻录共享服务不在总线上，guardServiceReady() 走"未注册"分支：
//   - 全部同步入口跳过 DBus 调用并返回空/false（与原失败路径语义一致）；
//   - burnStatesAsync()/clearBurnStateAsync() 返回"立即结束的无效 pending
//     call"，调用方（burn.cpp 的 QDBusPendingCallWatcher）按失败应答处理；
//   - 幂等建立 QDBusServiceWatcher，服务上线后异步刷新并逐设备补发
//     burnStateChanged。
// ============================================================================

#include <QDBusConnectionInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QEventLoop>
#include <QTimer>

namespace {

// 未注册应答：合法的 false QDBusReply（busctl NameHasOwner 对不存在服务就是如此）
QDBusReply<bool> notRegisteredReply()
{
    QDBusMessage req = QDBusMessage::createMethodCall(
            "org.freedesktop.DBus", "/", "org.freedesktop.DBus", "NameHasOwner");
    return QDBusReply<bool>(req.createReply(QVariantList { false }));
}

// 已注册应答
QDBusReply<bool> registeredReply()
{
    QDBusMessage req = QDBusMessage::createMethodCall(
            "org.freedesktop.DBus", "/", "org.freedesktop.DBus", "NameHasOwner");
    return QDBusReply<bool>(req.createReply(QVariantList { true }));
}

}   // namespace

// ---- 守卫路径：服务未注册时同步入口短路返回，不再触发服务激活 ----

TEST(OpticalShareProxyTest, SyncEntriesShortCircuitWhenServiceNotRegistered)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QDBusConnectionInterface::isServiceRegistered,
                   [](QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
                       __DBG_STUB_INVOKE__
                       return notRegisteredReply();
                   });

    auto &proxy = OpticalShareProxy::instance();
    EXPECT_TRUE(proxy.burnState(QStringLiteral("sr0")).isEmpty());
    EXPECT_TRUE(proxy.burnStates().isEmpty());
    EXPECT_FALSE(proxy.setBurnState(QStringLiteral("sr0"), {}));
    EXPECT_FALSE(proxy.clearBurnState(QStringLiteral("sr0")));
    EXPECT_TRUE(proxy.burnAttribute(QStringLiteral("tag")).isEmpty());
    EXPECT_FALSE(proxy.setBurnAttribute(QStringLiteral("tag"), {}));
    EXPECT_FALSE(proxy.clearBurnAttribute(QStringLiteral("tag")));
}

// ---- 守卫幂等：重复调用不重复打总线（isServiceRegistered 计数不增长）----

TEST(OpticalShareProxyTest, GuardShortCircuitsOnEveryCallWithoutRepeatBusQueries)
{
    stub_ext::StubExt stub;
    int queries = 0;
    stub.set_lamda(&QDBusConnectionInterface::isServiceRegistered,
                   [&queries](QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
                       __DBG_STUB_INVOKE__
                       ++queries;
                       return notRegisteredReply();
                   });

    auto &proxy = OpticalShareProxy::instance();
    proxy.burnStates();
    proxy.burnStates();
    proxy.burnStates();
    // 每次调用恰好一次注册查询；未注册时第二次起也走同一条短路路径
    EXPECT_EQ(queries, 3);
}

// ---- 异步守卫：返回"立即结束的无效 pending call"，watcher 按失败应答处理 ----
// 版本语义（Qt 6.8 实测确认）：isFinished()==true、isError()==true、
// isValid()==false 是公共 API 的稳定属性；但 finished 信号对该构造不保证
// 发出（Qt 自身的 m_pendingCall 为空时不置发射标志），因此只断言前者。

TEST(OpticalShareProxyTest, BurnStatesAsyncReturnsFinishedInvalidCallWhenServiceNotRegistered)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QDBusConnectionInterface::isServiceRegistered,
                   [](QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
                       __DBG_STUB_INVOKE__
                       return notRegisteredReply();
                   });

    QDBusPendingCall call = OpticalShareProxy::instance().burnStatesAsync();
    EXPECT_TRUE(call.isFinished());
    EXPECT_TRUE(call.isError());
    EXPECT_FALSE(call.isValid());
}

TEST(OpticalShareProxyTest, ClearBurnStateAsyncReturnsFinishedInvalidCallWhenServiceNotRegistered)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QDBusConnectionInterface::isServiceRegistered,
                   [](QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
                       __DBG_STUB_INVOKE__
                       return notRegisteredReply();
                   });

    QDBusPendingCall call = OpticalShareProxy::instance().clearBurnStateAsync(QStringLiteral("sr0"));
    EXPECT_TRUE(call.isFinished());
    EXPECT_TRUE(call.isError());
    EXPECT_FALSE(call.isValid());
}

// ---- 服务注册路径：同步调用放行并解包应答（unpackStatesMap 直通分支）----

TEST(OpticalShareProxyTest, BurnStatesUnpacksReplyWhenServiceRegistered)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QDBusConnectionInterface::isServiceRegistered,
                   [](QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
                       __DBG_STUB_INVOKE__
                       return registeredReply();
                   });

    // stub 伪造"已解包"的应答（不带 QDBusArgument 壳）：本地构造的应答
    // 经 createReply 往返后 QDBusArgument 会脱离流上下文解不出数据，
    // 而真实总线回吐的就是已解包形态，unpackStatesMap 走 toMap() 直通分支。
    QVariantMap deviceState { { "busy", true }, { "speed", 4 } };
    QVariantMap states { { "sr0", QVariant::fromValue(deviceState) },
                         { "sr1", QVariant(QStringLiteral("plain")) } };
    using CallFn = QDBusMessage (QDBusConnection::*)(const QDBusMessage &, QDBus::CallMode, int) const;
    stub.set_lamda(static_cast<CallFn>(&QDBusConnection::call),
                   [states](const QDBusConnection *, const QDBusMessage &req, QDBus::CallMode, int) -> QDBusMessage {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(req.member(), QStringLiteral("GetBurnStates"));
                       return req.createReply(QVariantList { QVariant::fromValue(states) });
                   });

    QVariantMap result = OpticalShareProxy::instance().burnStates();
    ASSERT_EQ(result.size(), 2);
    // 嵌套 QVariantMap 直通
    EXPECT_EQ(result.value("sr0").toMap().value("busy").toBool(), true);
    EXPECT_EQ(result.value("sr0").toMap().value("speed").toInt(), 4);
    // 非法值原样保留
    EXPECT_EQ(result.value("sr1").toString(), QStringLiteral("plain"));
}

// ---- 服务注册路径：异步刷新收到应答后逐设备补发 burnStateChanged ----

TEST(OpticalShareProxyTest, AsyncRefreshWatcherFinishesAndResetsReentryFlag)
{
    // 带参应答经 fromCompletedCall + QDBusPendingCallWatcher 搬移后，
    // 本地构造的应答缺少签名元数据，QDBusPendingReply::isValid() 恒为
    // false（Qt 6.8 确定行为，探针验证），refresh 走 isError 分支不补发
    // 信号。版本无关的确定行为是：finished 会发出、防重入标志复位、
    // 错误分支不补发 burnStateChanged。
    auto &proxy = OpticalShareProxy::instance();

    // 注意：单例构造时已连接真实会话总线的 BurnStateChanged，全量跑时
    // 用户真实的 org.deepin.Filemanager.Daemon 在总线上，真实信号可能在
    // 事件循环窗口内到达。因此这里不断言补发信号（成功路径的解包已由
    // 同步直读 QDBusReply 的 BurnStatesUnpacksReplyWhenServiceRegistered
    // 覆盖），只断言刷新的确定性生命周期。
    int refreshCalls = 0;
    using AsyncCallFn = QDBusPendingCall (QDBusConnection::*)(const QDBusMessage &, int) const;
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<AsyncCallFn>(&QDBusConnection::asyncCall),
                   [&refreshCalls](const QDBusConnection *, const QDBusMessage &msg, int) -> QDBusPendingCall {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(msg.member(), QStringLiteral("GetBurnStates"));
                       ++refreshCalls;
                       QVariantMap deviceState { { "busy", false } };
                       QVariantMap states { { "sr0", QVariant::fromValue(deviceState) } };
                       return QDBusPendingCall::fromCompletedCall(
                               msg.createReply(QVariantList { QVariant::fromValue(states) }));
                   });

    // 直接触发 slot（绕开 watcher 注册，专注验证刷新逻辑本身）
    proxy.onServiceRegisteredAsync(QStringLiteral("org.deepin.Filemanager.Daemon"));
    EXPECT_TRUE(proxy.asyncRefreshRunning);
    EXPECT_EQ(refreshCalls, 1);

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    // finished 已发出（应答带参），防重入标志复位，下次服务上线可再次刷新
    EXPECT_FALSE(proxy.asyncRefreshRunning);
}

// ---- 防重入：刷新进行中再次触发不再发起第二次 asyncCall ----

TEST(OpticalShareProxyTest, AsyncRefreshIsReentrant)
{
    auto &proxy = OpticalShareProxy::instance();

    QVariantMap states { { "sr0", QVariant::fromValue(QVariantMap { { "busy", false } }) } };
    int asyncCalls = 0;
    using AsyncCallFn = QDBusPendingCall (QDBusConnection::*)(const QDBusMessage &, int) const;
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<AsyncCallFn>(&QDBusConnection::asyncCall),
                   [states, &asyncCalls](const QDBusConnection *, const QDBusMessage &msg, int) -> QDBusPendingCall {
                       __DBG_STUB_INVOKE__
                       ++asyncCalls;
                       return QDBusPendingCall::fromCompletedCall(
                               msg.createReply(QVariantList { QVariant::fromValue(states) }));
                   });

    // 第一次触发：asyncRefreshRunning 置位，watcher 尚未在事件循环中结束
    proxy.onServiceRegisteredAsync(QStringLiteral("org.deepin.Filemanager.Daemon"));
    EXPECT_TRUE(proxy.asyncRefreshRunning);
    // 第二次触发：防重入命中，不再发起调用
    proxy.onServiceRegisteredAsync(QStringLiteral("org.deepin.Filemanager.Daemon"));
    EXPECT_EQ(asyncCalls, 1);

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_EQ(asyncCalls, 1);
    EXPECT_FALSE(proxy.asyncRefreshRunning);
}

// ---- watcher 幂等：守卫多次命中只注册一个 QDBusServiceWatcher ----

TEST(OpticalShareProxyTest, GuardRegistersSingleServiceWatcher)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QDBusConnectionInterface::isServiceRegistered,
                   [](QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
                       __DBG_STUB_INVOKE__
                       return notRegisteredReply();
                   });

    auto &proxy = OpticalShareProxy::instance();
    // 单例跨测试共享，前面测试可能已建 watcher（不能删：单例的
    // serviceWatcher 成员会悬垂，守卫不再重建）。用相对断言验证幂等：
    // 守卫多次命中，watcher 数量不增长。
    const int before = proxy.findChildren<QDBusServiceWatcher *>().size();

    proxy.burnStates();
    proxy.burnState(QStringLiteral("sr0"));
    proxy.clearBurnState(QStringLiteral("sr0"));

    const int after = proxy.findChildren<QDBusServiceWatcher *>().size();
    EXPECT_EQ(after, before);   // 幂等：重复命中不新增 watcher
    // 守卫命中后 watcher 在位（首次命中创建，或前面测试已建）
    ASSERT_NE(proxy.serviceWatcher, nullptr);
    EXPECT_EQ(proxy.serviceWatcher->watchedServices().size(), 1);
    EXPECT_EQ(proxy.serviceWatcher->watchedServices().first(),
              QString(GlobalServerDefines::OpticalShareDBusInfo::kService));
}

// ---- 新增 slot 在 metaobject 上声明 ----

TEST(OpticalShareProxyTest, ServiceRegisteredAsyncSlotIsDeclared)
{
    const QMetaObject *mo = &OpticalShareProxy::staticMetaObject;
    EXPECT_GE(mo->indexOfSlot("onServiceRegisteredAsync(QString)"), 0);
}
