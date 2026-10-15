// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_mock_dbus_env.h - disk-mount 插件测试共用的私有 DBus 总线 mock 环境
//
// dock-mount 插件的 DBus 调用全部同步取值（QDBusPendingReply::value()），
// 本地构造的完成应答缺少 D-Bus 签名无法通过解组校验，因此改为：
//   1. 启动私有 dbus-daemon（进程内 QProcess 托管，与真实会话总线隔离）；
//   2. 在私有总线上注册 org.deepin.Filemanager.Daemon.DeviceManager 与
//      org.freedesktop.Notifications 两个 mock 服务（槽内记录调用/回吐数据，
//      可发 D-Bus 错误应答）；
//   3. 测试固件用 stub_ext 把 QDBusConnection::sessionBus() 指到私有总线。
// 同线程同步取值由 Qt 在阻塞期间继续分发入站消息，无死锁（已探针验证）。

#ifndef MOCK_DBUS_ENV_H
#define MOCK_DBUS_ENV_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QVariantMap>
#include <QDBusConnection>

#include "stubext.h"

namespace ut {

inline const char kBlockId[] { "/org/freedesktop/UDisks2/block_devices/utblk" };
inline const char kBackingId[] { "/org/freedesktop/UDisks2/block_devices/utback" };
inline const char kProtoId[] { "utproto1" };
inline const char kRootDrive[] { "UTROOT" };

struct DevMngMock {
    // 行为旋钮
    QString blockId { kBlockId };
    QString backingId { kBackingId };
    QString protoId { kProtoId };
    QVariantMap blockData;
    QVariantMap backingData;
    QVariantMap protoData;
    QVariantMap defaultData;   // 未知块设备路径（含根盘探测）统一回此表
    QStringList blockIds;
    QStringList protoIds;
    bool failBlockList { false };
    bool failProtocolList { false };

    // 调用记录
    QStringList methods;
    QList<QVariantList> args;

    void reset();
};

struct NotifyMock {
    uint notifyId { 77 };
    QStringList methods;
    QList<QVariantList> args;

    void reset();
};

DevMngMock &devMngMock();
NotifyMock &notifyMock();

// 启动私有总线并注册 mock 服务（幂等；失败返回 false）
bool ensurePrivateBus();

// 将 QDBusConnection::sessionBus() 指到私有总线（固件 SetUp 中调用）
inline void installSessionBusStub(stub_ext::StubExt &stub)
{
    stub.set_lamda(&QDBusConnection::sessionBus, [] {
        __DBG_STUB_INVOKE__
        return QDBusConnection(QStringLiteral("ut-priv-bus"));
    });
}

}   // namespace ut

#endif   // MOCK_DBUS_ENV_H
