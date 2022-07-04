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
#include "mastered/masteredmediafileinfo.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>
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

    QString &&devFile { OpticalHelper::burnDestDevice(dest) };
    if (devFile.isEmpty())
        return {};

    QString &&mntPoint { DeviceUtils::getMountInfo(devFile) };
    if (mntPoint.isEmpty())
        return {};

    QString suffix { burnFilePath(dest) };
    return QUrl::fromLocalFile(mntPoint + suffix);
}

QString OpticalHelper::burnDestDevice(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::Scheme::kBurn || !url.path().contains(burnRxp(), &m))
        return {};
    return m.captured(1);
}

QString OpticalHelper::burnFilePath(const QUrl &url)
{
    QRegularExpressionMatch m;
    if (url.scheme() != Global::Scheme::kBurn || !url.path().contains(burnRxp(), &m))
        return {};
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

QUrl OpticalHelper::tansToLocalFile(const QUrl &in)
{
    Q_ASSERT(in.scheme() == Global::Scheme::kBurn);
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

void OpticalHelper::createStagingFolder(const QString &dev)
{
    if (!dev.startsWith("/dev/sr"))
        return;
    // Make sure the staging folder exists. Otherwise the staging watcher won't work.
    auto &&url { OpticalHelper::localStagingFile(dev) };
    if (!url.isValid())
        return;
    auto path { url.toLocalFile() };
    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
        QDir().mkpath(path);
}

bool OpticalHelper::isDupFileNameInPath(const QString &path, const QUrl &url)
{
    auto info { InfoFactory::create<AbstractFileInfo>(url) };
    if (!info || path.isEmpty())
        return false;
    QDir dir { path };
    if (!dir.exists())
        return false;

    QFileInfoList fileInfoList { dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs) };
    QString fileName { info->fileName() };
    for (const auto &info : fileInfoList) {
        if (info.fileName() == fileName)
            return true;
    }
    return false;
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

void OpticalHelper::pasteFilesHandle(const QList<QUrl> sources, const QUrl target, bool isCopy)
{
    dpfSlotChannel->push("dfmplugin_burn", "slot_PasteTo", sources, target, isCopy);
}

bool OpticalHelper::urlsToLocal(const QList<QUrl> &origins, QList<QUrl> *urls)
{
    if (!urls)
        return false;
    for (const QUrl &url : origins) {
        if (url.scheme() != OpticalHelper::scheme())
            return false;
        (*urls).push_back(tansToLocalFile(url));
    }
    return true;
}

bool OpticalHelper::cutFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    Q_UNUSED(windowId)
    Q_UNUSED(flags)
    pasteFilesHandle(sources, target, false);

    return true;
}

bool OpticalHelper::copyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;

    if (target.scheme() != scheme() || sources.first().scheme() != scheme())
        return false;

    Q_UNUSED(windowId)
    Q_UNUSED(flags)
    pasteFilesHandle(sources, target);
    return true;
}

bool OpticalHelper::moveToTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : sources) {
        QString backer { MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString() };
        if (backer.isEmpty())
            continue;
        if (!OpticalHelper::burnIsOnDisc(url))
            redirectedFileUrls.push_back(QUrl::fromLocalFile(backer));
    }

    dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles, windowId, redirectedFileUrls, flags, nullptr);
    return true;
}

bool OpticalHelper::openFileInPlugin(quint64 winId, QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        redirectedFileUrls << QUrl::fromLocalFile(MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString());
    }
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, winId, redirectedFileUrls);
    return true;
}

bool OpticalHelper::linkFile(const quint64 windowId, const QUrl url, const QUrl link, const bool force, const bool silence)
{
    if (url.scheme() != scheme())
        return false;

    QString backer { MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString() };
    if (backer.isEmpty())
        return false;

    QUrl redirectedFileUrl { QUrl::fromLocalFile(backer) };
    dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink, windowId, redirectedFileUrl, link, force, silence);
    return true;
}

bool OpticalHelper::writeUrlsToClipboard(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    if (action != ClipBoard::ClipboardAction::kCopyAction)
        return false;
    // only write file on disc
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        MasteredMediaFileInfo info(url);
        QUrl backerUrl { QUrl::fromLocalFile(info.extraProperties()["mm_backer"].toString()) };
        if (!OpticalHelper::localStagingRoot().isParentOf(backerUrl))
            redirectedFileUrls.push_back(backerUrl);
    }
    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, windowId, action, redirectedFileUrls);
    return true;
}

bool OpticalHelper::openFileInTerminal(const quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    const QString &currentDir = QDir::currentPath();

    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        QString backer { MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString() };
        if (backer.isEmpty())
            return false;
        redirectedFileUrls << QUrl::fromLocalFile(backer);
    }

    dpfSignalDispatcher->publish(GlobalEventType::kOpenInTerminal, windowId, redirectedFileUrls);
    QDir::setCurrent(currentDir);
    return true;
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
