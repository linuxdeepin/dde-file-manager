/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHFILEOPERATIONS_H
#define SEARCHFILEOPERATIONS_H

#include "dfmplugin_search_global.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"

DPSEARCH_BEGIN_NAMESPACE

class SearchFileOperations
{
public:
    static bool openFilesHandle(quint64 winId, const QList<QUrl> urls, const QString *error);
    static bool renameFileHandle(const quint64 winId, const QUrl oldUrl, const QUrl newUrl, QString *error);
    static bool openInTerminalHandle(const quint64 winId, const QList<QUrl> urls, QString *error);
    static bool writeToClipBoardHandle(const quint64 winId,
                                       const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                       const QList<QUrl> urls);
    static JobHandlePointer moveToTrashHandle(const quint64 winId,
                                              const QList<QUrl> sources,
                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    static JobHandlePointer deleteFilesHandle(const quint64 winId,
                                              const QList<QUrl> sources,
                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
};

DPSEARCH_END_NAMESPACE

#endif   // SEARCHFILEOPERATIONS_H
