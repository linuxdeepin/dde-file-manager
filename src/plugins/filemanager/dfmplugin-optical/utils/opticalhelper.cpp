/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "opticalhelper.h"

#include "dfm-base/base/urlroute.h"

#include <QCoreApplication>
#include <QRegularExpressionMatch>
#include <QStandardPaths>

#include <dfm-framework/framework.h>

DFMBASE_USE_NAMESPACE
DPOPTICAL_USE_NAMESPACE

// TODO(zhangs): rm define
#define DISCBURN_STAGING "discburn"
#define BURN_SEG_ONDISC "disc_files"
#define BURN_SEG_STAGING "staging_files"

QIcon OpticalHelper::icon()
{
    return QIcon::fromTheme(iconString());
}

QString OpticalHelper::iconString()
{
    return "media-optical-symbolic";
}

QUrl OpticalHelper::localStagingFile(const QUrl &dest)
{
    if (burnDestDevice(dest).length() == 0)
        return {};

    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"
                               + burnDestDevice(dest).replace('/', '_')
                               + burnFilePath(dest));
}

QUrl OpticalHelper::localStatgingFile(QString dev)
{
    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)   // ~/.cache
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"   // ~/.cache/deepin/discburn/
                               + dev.replace('/', '_'));
}

QString OpticalHelper::burnDestDevice(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != SchemeTypes::kBurn || !url.path().contains(burnRxp(), &m))
        return {};
    return m.captured(1);
}

QString OpticalHelper::burnFilePath(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != SchemeTypes::kBurn || !url.path().contains(burnRxp(), &m))
        return {};
    return m.captured(3);
}

QUrl OpticalHelper::tansToBurnFile(const QUrl &in)
{
    QRegularExpressionMatch m;
    QString cachepath = QRegularExpression::escape(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                                                   + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/");
    m = QRegularExpression(cachepath + "(.*)").match(in.path());
    Q_ASSERT(m.hasMatch());
    QString cpth = m.captured(1);
    m = QRegularExpression("(.*?)/(.*)").match(cpth);
    QString devid(m.captured(1));
    QString path(m.captured(2));
    if (!m.hasMatch()) {
        devid = cpth;
    }

    QUrl url;
    QString filePath { devid.replace('_', '/') + "/" BURN_SEG_STAGING "/" + path };
    url.setScheme(SchemeTypes::kBurn);
    url.setPath(filePath);

    return url;
}

DSB_FM_NAMESPACE::WindowsService *OpticalHelper::winServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WindowsService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WindowsService>(DSB_FM_NAMESPACE::WindowsService::name());
}

DSB_FM_NAMESPACE::TitleBarService *OpticalHelper::titleServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::TitleBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::TitleBarService>(DSB_FM_NAMESPACE::TitleBarService::name());
}

DSB_FM_NAMESPACE::WorkspaceService *OpticalHelper::workspaceServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WorkspaceService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WorkspaceService>(DSB_FM_NAMESPACE::WorkspaceService::name());
}

QRegularExpression OpticalHelper::burnRxp()
{
    static QRegularExpression rxp { "^(.*?)/(" BURN_SEG_ONDISC "|" BURN_SEG_STAGING ")(.*)$" };
    return rxp;
}
