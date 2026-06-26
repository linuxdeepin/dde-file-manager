// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLAUNCHUTILS_H
#define APPLAUNCHUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QString>
#include <QStringList>
#include <QMap>
#include <memory>

DFMBASE_BEGIN_NAMESPACE

// 定义应用启动器函数类型
using AppLaunchFunc = std::function<bool(const QString &desktopFile, const QStringList &filePaths)>;

// AM1 executeCommand 的 envVars 参数类型：a{ss} 字典。
// 命名与 org.desktopspec.ApplicationManager1 的 D-Bus introspection 注解
// (org.qtproject.QtDBus.QtTypeName.In4 = QStringMap) 一致。
// 用 using 别名（而非直接 QMap<QString,QString>）是为了让 Q_DECLARE_METATYPE
// 这样的函数式宏拿到单个 token，避免模板尖括号内的逗号被预处理器当作宏参数分隔符。
using QStringMap = QMap<QString, QString>;

class AppLaunchUtilsPrivate;
class AppLaunchUtils
{
    Q_DISABLE_COPY(AppLaunchUtils)
public:
    static AppLaunchUtils &instance();

    // 添加启动策略，优先级数字越小优先级越高
    void addStrategy(AppLaunchFunc launcher, int priority);

    // 启动应用
    bool launchApp(const QString &desktopFile, const QStringList &urlStrs);

    bool defaultLaunchApp(const QString &desktopFile, const QStringList &paths);

    // 通过 AM1 executeCommand 启动可执行程序（AppImage/二进制/脚本），纳入 AM1 实例管理。
    // type 须为 "shortcut"/"script"/"portablebinary"。
    // 返回 false 表示 D-Bus 不可用或调用失败，调用方应自行回退（如 QProcess::startDetached）。
    // 注意：此方法旁路策略列表，不经过 launchByDBus/launchByGio。
    bool executeCommand(const QString &program, const QStringList &args,
                        const QString &type, const QString &workdir = {},
                        const QString &runId = {},
                        const QStringMap &envVars = {});

private:
    AppLaunchUtils();

    std::unique_ptr<AppLaunchUtilsPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // APPLAUNCHUTILS_H
