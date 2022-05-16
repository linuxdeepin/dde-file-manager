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
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/dfm_global_defines.h"

#include <dfm-framework/framework.h>
#include <dfm-burn/dburn_global.h>

#include <QCoreApplication>
#include <QRegularExpressionMatch>
#include <QStandardPaths>

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

QUrl OpticalHelper::localStagingRoot()
{
    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/");
}

QUrl OpticalHelper::localStagingFile(const QUrl &dest)
{
    if (burnDestDevice(dest).isEmpty())
        return {};

    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"
                               + burnDestDevice(dest).replace('/', '_')
                               + burnFilePath(dest));
}

QUrl OpticalHelper::localStagingFile(QString dev)
{
    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)   // ~/.cache
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"   // ~/.cache/deepin/discburn/
                               + dev.replace('/', '_'));
}

QUrl OpticalHelper::localDiscFile(const QUrl &dest)
{
    using namespace GlobalServerDefines;

    QString devFile { OpticalHelper::burnDestDevice(dest) };
    if (devFile.isEmpty())
        return {};

    QString id { DeviceUtils::getBlockDeviceId(devFile) };
    auto &&map = DevProxyMng->queryBlockInfo(id);
    QString mntPoint { qvariant_cast<QString>(map[DeviceProperty::kMountPoint]) };
    if (mntPoint.isEmpty())
        return {};

    QString suffix { burnFilePath(dest) };
    return QUrl::fromLocalFile(mntPoint + suffix);
}

QString OpticalHelper::burnDestDevice(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::kBurn || !url.path().contains(burnRxp(), &m))
        return {};
    return m.captured(1);
}

QString OpticalHelper::burnFilePath(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::kBurn || !url.path().contains(burnRxp(), &m))
        return {};
    return m.captured(3);
}

bool OpticalHelper::burnIsOnDisc(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::kBurn || !url.path().contains(burnRxp(), &m)) {
        return false;
    }
    return m.captured(2) == BURN_SEG_ONDISC;
}

bool OpticalHelper::burnIsOnStaging(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::kBurn || !url.path().contains(burnRxp(), &m)) {
        return false;
    }
    return m.captured(2) == BURN_SEG_STAGING;
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
    url.setScheme(Global::kBurn);
    url.setPath(filePath);

    return url;
}

QUrl OpticalHelper::tansToLocalFile(const QUrl &in)
{
    Q_ASSERT(in.scheme() == Global::kBurn);
    QUrl url;

    if (burnIsOnDisc(in)) {
        url = localDiscFile(in);
    } else if (burnIsOnStaging(in)) {
        url = localStagingFile(in);
    }

    return url;
}

bool OpticalHelper::isSupportedUDFVersion(const QString &version)
{
    static const QStringList &&supported = {
        "1.02"
    };
    return supported.contains(version);
}

bool OpticalHelper::isSupportedUDFMedium(int type)
{
    static const QList<DFMBURN::MediaType> &&supportedMedium = {
        DFMBURN::MediaType::kDVD_R,
        DFMBURN::MediaType::kDVD_PLUS_R,
        DFMBURN::MediaType::kCD_R,
        DFMBURN::MediaType::kCD_RW
    };
    return supportedMedium.contains(DFMBURN::MediaType(type));
}

void OpticalHelper::createStagingFolder(const QString &path)
{
    // Make sure the staging folder exists. Otherwise the staging watcher won't work.
    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
        QDir().mkpath(path);
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

DSC_NAMESPACE::FileOperationsService *OpticalHelper::fileOperationsServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::FileOperationsService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
}

DSC_NAMESPACE::DelegateService *OpticalHelper::dlgateServIns()
{
    return delegateServIns;
}

DSC_NAMESPACE::MenuService *OpticalHelper::menuServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::MenuService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::MenuService>(DSC_NAMESPACE::MenuService::name());
}

QRegularExpression OpticalHelper::burnRxp()
{
    static QRegularExpression rxp { "^(.*?)/(" BURN_SEG_ONDISC "|" BURN_SEG_STAGING ")(.*)$" };
    return rxp;
}
