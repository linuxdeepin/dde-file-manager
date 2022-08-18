/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "fileoperatorhelper.h"
#include "utils/sidebarhelper.h"
#include "sidebarview.h"
#include "dfm-base/dfm_event_defines.h"
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
