// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileprevieweventreceiver.h"
#include "utils/previewdialogmanager.h"
#include "utils/previewhelper.h"

#include <dfm-framework/dpf.h>

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
    if(PreviewHelper::instance()->isPreviewEnabled())
        PreviewDialogManager::instance()->showPreviewDialog(windowId, selecturls, dirUrl);
}
