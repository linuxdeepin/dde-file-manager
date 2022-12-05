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
#include "opticaleventreceiver.h"
#include "utils/opticalhelper.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/utils/universalutils.h"

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
            auto fileInfo = InfoFactory::create<AbstractFileInfo>(curUrl);
            if (!fileInfo)
                break;
            QVariantMap map;
            map["CrumbData_Key_Url"] = curUrl;
            map["CrumbData_Key_DisplayText"] = fileInfo->displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
            mapGroup->push_front(map);
            if (fileInfo->parentUrl() == QUrl::fromLocalFile(QDir::homePath())) {
                mapGroup->front()["CrumbData_Key_IconName"] = "media-optical-symbolic";
                break;
            }
            curUrl = fileInfo->parentUrl();
        }
        return true;
    }

    return false;
}

bool OpticalEventReceiver::handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (toUrl.scheme() == Global::Scheme::kBurn) {
        QString &&path { OpticalHelper::burnFilePath(toUrl) };
        if (path.isEmpty() || path == "/") {
            dpfSignalDispatcher->publish(GlobalEventType::kCopy,
                                         0,
                                         fromUrls,
                                         toUrl,
                                         AbstractJobHandler::JobFlag::kNoHint, nullptr);
        }
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

OpticalEventReceiver::OpticalEventReceiver(QObject *parent)
    : QObject(parent)
{
}
