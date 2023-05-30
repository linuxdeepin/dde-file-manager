// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticaleventreceiver.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_optical;
DFMBASE_USE_NAMESPACE

OpticalEventReceiver &OpticalEventReceiver::instance()
{
    static OpticalEventReceiver ins;
    return ins;
}

bool OpticalEventReceiver::handleDeleteFilesShortcut(quint64, const QList<QUrl> &urls)
{
    auto iter = std::find_if(urls.cbegin(), urls.cend(), [](const QUrl &url) {
        return OpticalHelper::burnIsOnDisc(url);
    });
    if (iter != urls.cend()) {
        qInfo() << "delete event is blocked, trying to delete disc burn:///*";
        return true;
    }
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

bool OpticalEventReceiver::handleTabClosable(const QUrl &currentUrl, const QUrl &rootUrl)
{
    const auto &scheme { OpticalHelper::scheme() };
    if (currentUrl.scheme() != scheme || rootUrl.scheme() != scheme)
        return false;

    if (OpticalHelper::burnIsOnStaging(currentUrl)) {
        const QString &rootDev { OpticalHelper::burnDestDevice(rootUrl) };
        const QString &curDev { OpticalHelper::burnDestDevice(currentUrl) };
        if (rootDev == curDev) {
            qInfo() << "Close tab: " << currentUrl;
            return true;
        }
    }

    return false;
}

OpticalEventReceiver::OpticalEventReceiver(QObject *parent)
    : QObject(parent)
{
}
