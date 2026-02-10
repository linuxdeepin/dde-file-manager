// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticaleventreceiver.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_optical;
DFMBASE_USE_NAMESPACE

OpticalEventReceiver &OpticalEventReceiver::instance()
{
    static OpticalEventReceiver ins;
    return ins;
}

bool OpticalEventReceiver::handleDeleteFilesShortcut(quint64, const QList<QUrl> &urls, const QUrl &rootUrl)
{
    if (!DevProxyMng->isFileFromOptical(rootUrl.toLocalFile()))
        return false;

    auto iter = std::find_if(urls.cbegin(), urls.cend(), [](const QUrl &url) {
        return OpticalHelper::burnIsOnDisc(url);
    });
    if (iter != urls.cend()) {
        fmInfo() << "delete event is blocked, trying to delete disc burn:///*";
        return true;
    }

    // 仅 PW 光盘根目录支持
    if (isContainPWSubDirFile(urls))
        return true;

    return false;
}

bool OpticalEventReceiver::handleCheckDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action)
{
    if (urls.isEmpty())
        return false;
    if (!urlTo.isValid())
        return false;
    if (!action)
        return false;

    if (urlTo.scheme() == DFMBASE_NAMESPACE::Global::Scheme::kBurn) {
        QString &&path { OpticalHelper::burnFilePath(urlTo) };
        // only allow drop file to root path now
        if (path.isEmpty() || path == "/") {
            *action = Qt::CopyAction;
            return true;
        }
    }

    return false;
}

bool OpticalEventReceiver::handleMoveToTrashShortcut(quint64 winId, const QList<QUrl> &urls, const QUrl &rootUrl)
{
    Q_UNUSED(winId);

    if (!DevProxyMng->isFileFromOptical(rootUrl.toLocalFile()))
        return false;

    if (isContainPWSubDirFile(urls))
        return true;

    return false;
}

bool OpticalEventReceiver::handleCutFilesShortcut(quint64 winId, const QList<QUrl> &urls, const QUrl &rootUrl)
{
    Q_UNUSED(winId);

    if (!DevProxyMng->isFileFromOptical(rootUrl.toLocalFile()))
        return false;

    // 仅 PW 光盘根目录支持
    if (isContainPWSubDirFile(urls))
        return true;

    return false;
}

bool OpticalEventReceiver::handlePasteFilesShortcut(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to)
{
    Q_UNUSED(winId);
    Q_UNUSED(fromUrls);

    const QString &path { to.toLocalFile() };
    if (!DevProxyMng->isFileFromOptical(path))
        return false;

    QString dev { DeviceUtils::getMountInfo(path, false) };

    // 仅根目录支持 paste
    if (dev.isEmpty()) {
        QString curMnt { OpticalHelper::findMountPoint(path) };
        dev = DeviceUtils::getMountInfo(curMnt, false);
        if (DeviceUtils::isPWUserspaceOpticalDiscDev(dev))
            return true;
    }

    return false;
}

bool OpticalEventReceiver::sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);
    if (url.scheme() == DFMBASE_NAMESPACE::Global::Scheme::kBurn) {
        QUrl curUrl(url);
        while (true) {
            auto fileInfo = InfoFactory::create<FileInfo>(curUrl);
            if (!fileInfo)
                break;
            QVariantMap map;
            map["CrumbData_Key_Url"] = curUrl;
            map["CrumbData_Key_DisplayText"] = fileInfo->displayOf(DisPlayInfoType::kFileDisplayName);
            mapGroup->push_front(map);
            if (fileInfo->urlOf(UrlInfoType::kParentUrl) == QUrl::fromLocalFile(QDir::homePath())) {
                mapGroup->front()["CrumbData_Key_IconName"] = "media-optical-symbolic";
                break;
            }
            curUrl = fileInfo->urlOf(UrlInfoType::kParentUrl);
        }
        return true;
    }

    return false;
}

bool OpticalEventReceiver::handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    QList<QUrl> transformedUrls;
    UniversalUtils::urlsTransformToLocal(fromUrls, &transformedUrls);
    if (toUrl.scheme() == Global::Scheme::kBurn) {
        QString &&path { OpticalHelper::burnFilePath(toUrl) };
        if (path.isEmpty() || path == "/") {
            dpfSignalDispatcher->publish(GlobalEventType::kCopy,
                                         0,
                                         transformedUrls,
                                         toUrl,
                                         AbstractJobHandler::JobFlag::kNoHint, nullptr);
        }
        return true;
    }

    return false;
}

bool OpticalEventReceiver::handleBlockShortcutPaste(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to)
{
    Q_UNUSED(winId)
    Q_UNUSED(fromUrls)

    if (to.scheme() == OpticalHelper::scheme()) {
        const QString &dev { OpticalHelper::burnDestDevice(to) };
        const QUrl &rootUrl { OpticalHelper::discRoot(dev) };
        // only root url enable paste
        if (rootUrl.isValid() && !UniversalUtils::urlEquals(to, rootUrl))
            return true;
    }

    return false;
}

bool OpticalEventReceiver::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (url.scheme() == OpticalHelper::scheme()) {
        const QString &dev { OpticalHelper::burnDestDevice(url) };
        const QUrl &rootUrl { OpticalHelper::discRoot(dev) };
        if (rootUrl.isValid() && UniversalUtils::urlEquals(url, rootUrl)) {
            *iconName = "media-optical";
            return true;
        }
    }

    return false;
}

bool OpticalEventReceiver::handleTabCloseable(const QUrl &currentUrl, const QUrl &rootUrl)
{
    const auto &scheme { OpticalHelper::scheme() };
    if (currentUrl.scheme() != scheme || rootUrl.scheme() != scheme)
        return false;

    // 当前所在的目录正在待刻录的目录中（staging），而一个磁盘的目录被删除了（disc）这意味着
    // 光盘被卸载， 因此需要关闭当前tab
    if (OpticalHelper::burnIsOnStaging(currentUrl) && OpticalHelper::burnIsOnDisc(rootUrl)) {
        const QString &rootDev { OpticalHelper::burnDestDevice(rootUrl) };
        const QString &curDev { OpticalHelper::burnDestDevice(currentUrl) };
        if (rootDev == curDev) {
            fmInfo() << "Close tab: " << currentUrl;
            return true;
        }
    }

    return false;
}

bool OpticalEventReceiver::isContainPWSubDirFile(const QList<QUrl> &urls)
{
    // 仅 PW 光盘根目录支持
    if (std::any_of(urls.begin(), urls.end(), [](const QUrl &url) {
            const QString &directory {
                url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile()
            };
            const QString &curMnt { OpticalHelper::findMountPoint(directory) };
            if (curMnt == directory)
                return false;
            const QString &dev { DeviceUtils::getMountInfo(curMnt, false) };
            if (directory.startsWith(curMnt) && DeviceUtils::isPWUserspaceOpticalDiscDev(dev))
                return true;
            return false;
        }))
        return true;

    return false;
}

OpticalEventReceiver::OpticalEventReceiver(QObject *parent)
    : QObject(parent)
{
}
