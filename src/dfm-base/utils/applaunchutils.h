// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLAUNCHUTILS_H
#define APPLAUNCHUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QString>
#include <QStringList>
#include <memory>

DFMBASE_BEGIN_NAMESPACE

// 定义应用启动器函数类型
using AppLaunchFunc = std::function<bool(const QString &desktopFile, const QStringList &filePaths)>;

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

private:
    AppLaunchUtils();

    std::unique_ptr<AppLaunchUtilsPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // APPLAUNCHUTILS_H
