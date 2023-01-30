/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "hiddenfilefilter.h"

#include "base/schemefactory.h"
#include "base/urlroute.h"
#include "file/local/localfileinfo.h"

#include <dfm-framework/dpf.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

static DFMLocalFileInfoPointer createFileInfo(const QUrl &url)
{
    QString errString;
    auto itemInfo = InfoFactory::create<LocalFileInfo>(url, true, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        qInfo() << "create LocalFileInfo error: " << errString << url;
        return nullptr;
    }

    return itemInfo;
}

HiddenFileFilter::HiddenFileFilter()
    : QObject(), ModelDataHandler()
{
    updateFlag();
    dpfSignalDispatcher->subscribe("ddplugin_canvas", "signal_CanvasModel_HiddenFlagChanged", this, &HiddenFileFilter::hiddenFlagChanged);
}

HiddenFileFilter::~HiddenFileFilter()
{
    dpfSignalDispatcher->unsubscribe("ddplugin_canvas", "signal_CanvasModel_HiddenFlagChanged", this, &HiddenFileFilter::hiddenFlagChanged);
}

void HiddenFileFilter::refreshModel()
{
    dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionModel_Refresh", false, 50);
}

bool HiddenFileFilter::acceptInsert(const QUrl &url)
{
    if (showHiddenFiles())
        return true;

    if (auto info = createFileInfo(url)) {
        //! fouce refresh
        info->refresh();
        return !info->isAttributes(OptInfoType::kIsHidden);
    }

    return true;
}

QList<QUrl> HiddenFileFilter::acceptReset(const QList<QUrl> &urls)
{
    if (showHiddenFiles())
        return urls;

    auto allUrl = urls;
    for (auto itor = allUrl.begin(); itor != allUrl.end();) {
        auto info = createFileInfo(*itor);
        if (info) {
            //! file cached will not change hidden propety when .hidden file changed.
            //! need to fouce refresh
            //! maybe it need be fixed in dfm-io
            info->refresh();
            if (info->isAttributes(OptInfoType::kIsHidden)) {
                itor = allUrl.erase(itor);
                continue;
            }
        }
        ++itor;
    }

    return allUrl;
}

bool HiddenFileFilter::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    return acceptInsert(newUrl);
}

bool HiddenFileFilter::acceptUpdate(const QUrl &url)
{
    // the filemanager hidden attr changed.
    // get file that removed form .hidden if do not show hidden file.
    if (!showHiddenFiles() && url.fileName() == ".hidden") {
        qDebug() << "refresh by hidden changed.";
        refreshModel();
        return false;
    }

    return true;
}

void HiddenFileFilter::updateFlag()
{
    show = dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasModel_ShowHiddenFiles").toBool();
}

void HiddenFileFilter::hiddenFlagChanged(bool showHidden)
{
    qDebug() << "refresh by canvas hidden flag changed." << showHidden;
    show = showHidden;
    refreshModel();
}
