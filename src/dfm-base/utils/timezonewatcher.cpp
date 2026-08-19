// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timezonewatcher.h"

#include <QFile>
#include <QFileInfo>
#include <QDBusConnection>

#include <ctime>

namespace dfmbase {

TimezoneWatcher::TimezoneWatcher(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

TimezoneWatcher &TimezoneWatcher::instance()
{
    static TimezoneWatcher w;
    return w;
}

void TimezoneWatcher::init()
{
    if (m_initialized)
        return;

    // 1. Initialize timezone
    readAndSetTimezone();

    // 2. Connect D-Bus signals
    connectDbusSignals();

    m_initialized = true;

    qCInfo(logDFMBase) << "TimezoneWatcher initialized, timezone:" << m_currentTimezone;
}

QString TimezoneWatcher::currentTimezone() const
{
    return m_currentTimezone;
}

bool TimezoneWatcher::isInitialized() const
{
    return m_initialized;
}

void TimezoneWatcher::readAndSetTimezone()
{
    QString tz;

    // 1. Try /etc/timezone first
    tz = readTimezoneFromEtcTimezone();

    // 2. Fallback: /etc/localtime symlink
    if (tz.isEmpty()) {
        tz = readTimezoneFromLocaltime();
    }

    // 3. Final fallback to UTC
    if (tz.isEmpty())
        tz = QStringLiteral("UTC");

    // 4. Update if changed
    if (tz != m_currentTimezone) {
        m_currentTimezone = tz;
        qputenv("TZ", tz.toUtf8());
        tzset();  // Apply immediately
        emit timezoneChanged(tz);
        qCInfo(logDFMBase) << "Timezone updated to:" << tz;
    }
}

QString TimezoneWatcher::readTimezoneFromEtcTimezone()
{
    QFile tzFile(QStringLiteral("/etc/timezone"));
    if (tzFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString tz = QString::fromUtf8(tzFile.readAll()).trimmed();
        if (!tz.isEmpty())
            return tz;
    }
    return QString();
}

QString TimezoneWatcher::readTimezoneFromLocaltime()
{
    QFileInfo info(QStringLiteral("/etc/localtime"));
    if (info.isSymLink()) {
        QString path = info.symLinkTarget();
        int pos = path.lastIndexOf(QStringLiteral("/zoneinfo/"));
        if (pos >= 0)
            return path.mid(pos + 10);
    }
    return QString();
}

void TimezoneWatcher::connectDbusSignals()
{
    // 订阅 org.freedesktop.timedate1 的 PropertiesChanged 信号。
    //
    // 这里不传 service（首参留空），原因是：若传入 well-known name
    // "org.freedesktop.timedate1"，Qt 内部会同步调用 GetNameOwner 解析为
    // unique name 再下发 AddMatch；当该服务未注册时该调用会阻塞直至
    // D-Bus 默认超时（约 25s），导致文件管理器启动卡顿 25 秒。
    //
    // 留空 service 生成的 match rule 不含 sender 过滤、且不做 GetNameOwner，
    // 因而是非阻塞的；再通过 pin 住 object path "/org/freedesktop/timedate1"
    // 与 interface "org.freedesktop.DBus.Properties" + member "PropertiesChanged"
    // 将匹配范围收敛到该对象路径——而该路径仅由 timedate1 持有，故只有
    // timedate1 发出的 PropertiesChanged 才会投递到本 slot。
    //
    // 这同时解决了两个问题：
    //   1) 不再因服务未就绪而阻塞启动（无 25s 卡顿）；
    //   2) 不再因 checkDbusService 守卫而静默放弃订阅——服务随时上线后，
    //      其发出的 PropertiesChanged 仍会被投递。
    //
    // 注意：slot 内仍按 interface == "org.freedesktop.timedate1" 二次过滤。
    QDBusConnection::systemBus().connect(
                QString(),   // empty service: non-blocking, no GetNameOwner
                QStringLiteral("/org/freedesktop/timedate1"),
                QStringLiteral("org.freedesktop.DBus.Properties"),
                QStringLiteral("PropertiesChanged"),
                this,
                SLOT(onPropertiesChanged(QString,QVariantMap,QStringList)));
}

void TimezoneWatcher::onPropertiesChanged(const QString &interface,
                                      const QVariantMap &changed,
                                      const QStringList &invalidated)
{
    // Only handle org.freedesktop.timedate1 interface
    if (interface == QStringLiteral("org.freedesktop.timedate1")) {
        // PropertiesChanged 的 changed 是已重新获取的属性，invalidated 是已失效
        // 需重新读取的属性。Timezone 可能出现在任一列表中，两者都需处理。
        if (changed.contains(QStringLiteral("Timezone"))
                || invalidated.contains(QStringLiteral("Timezone"))) {
            qCInfo(logDFMBase) << "Timezone D-Bus properties changed, re-reading timezone";
            readAndSetTimezone();
        }
    }
}

}   // namespace dfmbase
