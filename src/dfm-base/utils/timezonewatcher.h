// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEZONEWATCHER_H
#define TIMEZONEWATCHER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>

namespace dfmbase {

/*!
 * \class TimezoneWatcher
 * \brief 时区监视器，用于监听系统时区变化并自动更新 TZ 环境变量
 *
 * 解决 Qt QDateTime::currentDateTime() 每次调用都会 stat("/etc/localtime") 的问题
 *
 * 使用方法:
 *   TimezoneWatcher::instance().init();
 *
 * 监视机制:
 *   - D-Bus: org.freedesktop.timedate1 PropertiesChanged 信号（服务启动后自动生效）
 *   - 文件系统 fallback: /etc/timezone 和 /etc/localtime
 *   - 最终 fallback: UTC
 */
class TimezoneWatcher : public QObject
{
    Q_OBJECT

public:
    /*! 获取单例实例
     * \return TimezoneWatcher 实例引用
     */
    static TimezoneWatcher &instance();

    /*! 初始化时区信息
     * 在应用程序启动时调用，设置 TZ 环境变量并启动监视
     */
    void init();

    /*! 获取当前时区
     * \return 当前时区字符串，如 "Asia/Shanghai"
     */
    QString currentTimezone() const;

    /*! 是否已初始化
     * \return true 表示已成功初始化
     */
    bool isInitialized() const;

Q_SIGNALS:
    /*! 时区变化信号
     * \param timezone 新的时区字符串
     */
    void timezoneChanged(const QString &timezone);

private Q_SLOTS:
    /*! 处理 D-Bus PropertiesChanged 信号
     * \param interface D-Bus 接口名
     * \param changed 变化的属性
     * \param invalidated 无效的属性列表
     */
    void onPropertiesChanged(const QString &interface,
                          const QVariantMap &changed,
                          const QStringList &invalidated);

private:
    explicit TimezoneWatcher(QObject *parent = nullptr);
    ~TimezoneWatcher() override = default;

    /*! 读取时区信息并设置 TZ 环境变量
     * 优先从 /etc/timezone 读取，回退到 /etc/localtime 符号链接，最终 fallback 到 UTC
     */
    void readAndSetTimezone();

    /*! 读取 /etc/timezone 文件
     * \return 时区字符串，失败返回空
     */
    QString readTimezoneFromEtcTimezone();

    /*! 从 /etc/localtime 符号链接读取时区
     * \return 时区字符串，失败返回空
     */
    QString readTimezoneFromLocaltime();

    /*! 连接到 D-Bus 信号
     */
    void connectDbusSignals();

private:
    QString m_currentTimezone;
    bool m_initialized;
};

}   // namespace dfmbase

#endif   // TIMEZONEWATCHER_H
