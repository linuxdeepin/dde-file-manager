// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMSERVICEMANAGER_H
#define SYSTEMSERVICEMANAGER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QString>

namespace dfmbase {

/**
 * @class SystemServiceManager
 * @brief 通用系统服务管理器
 *
 * 基于 systemd D-Bus 接口的服务管理器，提供统一的系统服务控制能力。
 * 直接调用 org.freedesktop.systemd1.Manager，systemd 通过 PolicyKit 自动处理权限。
 *
 * 优势：
 * - 标准化：使用 systemd 官方 D-Bus 接口
 * - 安全性：systemd 自带 PolicyKit 策略保证权限安全
 * - 可复用：dfm-base 中的实现可被所有插件使用
 * - 易维护：无需维护自定义后端服务
 *
 * 架构：
 * Frontend → systemd D-Bus API → PolicyKit(systemd) → systemd
 */
class SystemServiceManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SystemServiceManager)

public:
    /**
     * @brief 服务操作类型
     */
    enum ServiceAction {
        Start,   // 启动服务（临时）
        Stop,   // 停止服务
        Restart,   // 重启服务
        Enable,   // 开机自启（持久化）
        Disable   // 禁用自启
    };

    /**
     * @brief 获取单例实例
     * @return 单例引用
     */
    static SystemServiceManager &instance();

    /**
     * @brief 检查服务是否正在运行
     * @param serviceName 服务名（如 "smbd.service"）
     * @return true 表示服务正在运行
     */
    bool isServiceRunning(const QString &serviceName);

    /**
     * @brief 启动服务（临时，重启后失效）
     * @param serviceName 服务名
     * @return true 表示操作成功
     */
    bool startService(const QString &serviceName);

    /**
     * @brief 启用服务开机自启（持久化）
     * @param serviceName 服务名
     * @return true 表示操作成功
     */
    bool enableServiceNow(const QString &serviceName);

private:
    explicit SystemServiceManager(QObject *parent = nullptr);
    ~SystemServiceManager() override = default;

    /**
     * @brief 将服务名转换为 systemd unit 路径
     * @param serviceName 服务名（如 "smbd.service"）
     * @return unit 路径（如 "/org/freedesktop/systemd1/unit/smbd_2eservice"）
     */
    QString unitPathFromName(const QString &serviceName);
};

}   // namespace dfmbase

#endif   // SYSTEMSERVICEMANAGER_H
