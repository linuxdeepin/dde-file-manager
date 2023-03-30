// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hiddenfilefilter.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/file/local/syncfileinfo.h"

#include <dfm-framework/dpf.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

static FileInfoPointer createFileInfo(const QUrl &url)
{
    QString errString;
    auto itemInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        qInfo() << "create FileInfo error: " << errString << url;
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
