// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperatorhelper.h"
#include "utils/sidebarhelper.h"
#include "sidebarview.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <dfm-framework/dpf.h>

DFMBASE_USE_NAMESPACE
DPSIDEBAR_USE_NAMESPACE

FileOperatorHelper *FileOperatorHelper::instance()
{
    static FileOperatorHelper ins;
    return &ins;
}

void FileOperatorHelper::pasteFiles(quint64 windowId, const QList<QUrl> &srcUrls, const QUrl &targetUrl, const Qt::DropAction &action)
{
    if (action == Qt::MoveAction) {
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile,
                                     windowId,
                                     srcUrls,
                                     targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else {
        // default is copy file
        dpfSignalDispatcher->publish(GlobalEventType::kCopy,
                                     windowId,
                                     srcUrls,
                                     targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
    }
}

FileOperatorHelper::FileOperatorHelper(QObject *parent)
    : QObject(parent)
{
}
