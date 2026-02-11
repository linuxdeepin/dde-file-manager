// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalhelper.h"
#include "mastered/masteredmediafileinfo.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/event/event.h>

#include <dfm-burn/dburn_global.h>

#include <QCoreApplication>
#include <QRegularExpressionMatch>
#include <QStandardPaths>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;

// TODO(zhangs): rm define
#define DISCBURN_STAGING "discburn"
#define BURN_SEG_ONDISC "disc_files"
#define BURN_SEG_STAGING "staging_files"

OpticalHelper *OpticalHelper::instance()
{
    static OpticalHelper instance;
    return &instance;
}

QString OpticalHelper::scheme()
{
    return Global::Scheme::kBurn;
}

QIcon OpticalHelper::icon()
{
    return QIcon::fromTheme(iconString());
}

QString OpticalHelper::iconString()
{
    return "media-optical-symbolic";
}

QUrl OpticalHelper::discRoot(const QString &dev)
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath(dev + "/" BURN_SEG_ONDISC "/");
    return url;
}

QUrl OpticalHelper::localStagingRoot()
{
    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/");
}

QUrl OpticalHelper::localStagingFile(const QUrl &dest)
{
    if (burnDestDevice(dest).isEmpty()) {
        fmWarning() << "Cannot get staging file - no destination device for:" << dest;
        return {};
    }

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

    QString &&devFile { OpticalHelper::burnDestDevice(dest) };
    if (devFile.isEmpty()) {
        fmWarning() << "Cannot get disc file - no destination device for:" << dest;
        return {};
    }

    QString &&mntPoint { DeviceUtils::getMountInfo(devFile) };
    if (mntPoint.isEmpty()) {
        fmWarning() << "Cannot get disc file - no mount point for device:" << devFile;
        return {};
    }

    QString suffix { burnFilePath(dest) };
    return QUrl::fromLocalFile(mntPoint + suffix);
}

QString OpticalHelper::burnDestDevice(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::Scheme::kBurn || !url.path().contains(burnRxp(), &m)) {
        fmDebug() << "URL is not a valid burn destination:" << url;
        return {};
    }
    return m.captured(1);
}

QString OpticalHelper::burnFilePath(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::Scheme::kBurn || !url.path().contains(burnRxp(), &m)) {
        fmDebug() << "URL is not a valid burn URL for file path extraction:" << url;
        return {};
    }
    return m.captured(3);
}

bool OpticalHelper::burnIsOnDisc(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::Scheme::kBurn || !url.path().contains(burnRxp(), &m)) {
        return false;
    }
    return m.captured(2) == BURN_SEG_ONDISC;
}

bool OpticalHelper::burnIsOnStaging(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::Scheme::kBurn || !url.path().contains(burnRxp(), &m)) {
        fmDebug() << "URL is not a valid burn URL for staging check:" << url;
        return false;
    }
    return m.captured(2) == BURN_SEG_STAGING;
}

QUrl OpticalHelper::tansToBurnFile(const QUrl &in)
{
    QRegularExpressionMatch m;
    QString cachepath = QRegularExpression::escape(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                                                   + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/");
    m = QRegularExpression(cachepath + "([\\s\\S]*)").match(in.path());
    Q_ASSERT(m.hasMatch());
    QString cpth = m.captured(1);
    m = QRegularExpression("([\\s\\S]*?)/([\\s\\S]*)").match(cpth);
    QString devid(m.captured(1));
    QString path(m.captured(2));
    if (!m.hasMatch())
        devid = cpth;

    QUrl url;
    QString filePath { devid.replace('_', '/') + "/" BURN_SEG_STAGING "/" + path };
    url.setScheme(Global::Scheme::kBurn);
    url.setPath(filePath);

    return url;
}

QUrl OpticalHelper::transDiscRootById(const QString &id)
{
    if (!id.contains(QRegularExpression("sr[0-9]*$"))) {
        fmWarning() << "Invalid device ID format for disc root transformation:" << id;
        return {};
    }
    const QString &&volTag { id.mid(id.lastIndexOf("/") + 1) };
    return QUrl(QString("burn:///dev/%1/disc_files/").arg(volTag));
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

void OpticalHelper::createStagingFolder(const QString &dev)
{
    if (!dev.startsWith("/dev/sr")) {
        fmDebug() << "Device is not an optical device, skipping staging folder creation:" << dev;
        return;
    }
    // Make sure the staging folder exists. Otherwise the staging watcher won't work.
    auto &&url { OpticalHelper::localStagingFile(dev) };
    if (!url.isValid()) {
        fmWarning() << "Invalid staging URL, cannot create folder for device:" << dev;
        return;
    }
    auto path { url.toLocalFile() };
    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
        QDir().mkpath(path);
}

bool OpticalHelper::isDupFileNameInPath(const QString &path, const QUrl &url)
{
    auto info { InfoFactory::create<FileInfo>(url) };
    if (!info || path.isEmpty()) {
        fmDebug() << "Cannot check duplicate - invalid info or empty path";
        return false;
    }
    QDir dir { path };
    if (!dir.exists()) {
        fmDebug() << "Directory does not exist for duplicate check:" << path;
        return false;
    }

    QFileInfoList fileInfoList { dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs) };
    QString fileName { info->nameOf(NameInfoType::kFileName) };
    for (const auto &info : fileInfoList) {
        if (info.fileName() == fileName)
            return true;
    }
    return false;
}

bool OpticalHelper::isBurnEnabled()
{
    const auto &&ret = DConfigManager::instance()->value("org.deepin.dde.file-manager.burn", "burnEnable");
    return ret.isValid() ? ret.toBool() : true;
}

QStringList OpticalHelper::allOpticalDiscMountPoints()
{
    using namespace GlobalServerDefines;
    QStringList mnts;
    const auto &discIdGroup { DevProxyMng->getAllBlockIds(
            DeviceQueryOption::kOptical | DeviceQueryOption::kMounted) };

    std::transform(discIdGroup.begin(), discIdGroup.end(), std::back_inserter(mnts),
                   [](const auto &id) {
                       const auto &map { DevProxyMng->queryBlockInfo(id) };
                       return map.value(DeviceProperty::kMountPoints).toString();
                   });

    return mnts;
}

QString OpticalHelper::findMountPoint(const QString &path)
{
    const auto &mnts { OpticalHelper::allOpticalDiscMountPoints() };
    for (const auto &mnt : mnts) {
        if (path.startsWith(mnt)) {
            fmDebug() << "Found mount point:" << mnt << "for path:" << path;
            return mnt;
        }
    }

    fmDebug() << "No mount point found for path:" << path;
    return {};
}

bool OpticalHelper::isTransparent(const QUrl &url, Global::TransparentStatus *status)
{
    if (url.scheme() == OpticalHelper::scheme()) {
        if (!burnIsOnDisc(url))
            *status = Global::TransparentStatus::kTransparent;
        return true;
    }

    return false;
}

OpticalHelper::OpticalHelper(QObject *parent)
    : QObject(parent)
{
}

QRegularExpression OpticalHelper::burnRxp()
{
    static QRegularExpression rxp { "^([\\s\\S]*?)/(" BURN_SEG_ONDISC "|" BURN_SEG_STAGING ")([\\s\\S]*)$" };
    return rxp;
}
