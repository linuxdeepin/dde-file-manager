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
#include "previewfileoperation.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QList>

#include <QUrl>

DFMBASE_USE_NAMESPACE
DPFILEPREVIEW_USE_NAMESPACE
PreviewFileOperation::PreviewFileOperation(QObject *parent)
    : QObject(parent)
{
}

void PreviewFileOperation::openFileHandle(quint64 winID, const QUrl &url)
{
    QList<QUrl> urls;
    urls << url;
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, winID, urls);
}
