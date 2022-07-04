/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "fileprevieweventreceiver.h"
#include "utils/previewdialogmanager.h"

#include <dfm-framework/framework.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_filepreview;
FilePreviewEventReceiver::FilePreviewEventReceiver(QObject *parent)
    : QObject(parent)
{
}

FilePreviewEventReceiver *FilePreviewEventReceiver::instance()
{
    static FilePreviewEventReceiver receiver;
    return &receiver;
}

void FilePreviewEventReceiver::connectService()
{
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPFILEPREVIEW_NAMESPACE), "slot_PreviewDialog_Show", this, &FilePreviewEventReceiver::showFilePreview);
}

void FilePreviewEventReceiver::showFilePreview(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl)
{
    PreviewDialogManager::instance()->showPreviewDialog(windowId, selecturls, dirUrl);
}
