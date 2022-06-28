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
#ifndef FILEMANIPULATION_H
#define FILEMANIPULATION_H
#include "dfmplugin_vault_global.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/dfm_global_defines.h"

#include <QObject>

namespace dfmplugin_vault {
class FileManipulation : public QObject
{
    Q_OBJECT
public:
    explicit FileManipulation(QObject *parent = nullptr);

public:
    static bool openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error);

    static bool writeToClipBoardHandle(const quint64 windowId,
                                       const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                       const QList<QUrl> urls);

    static JobHandlePointer moveToTrashHandle(const quint64 windowId, const QList<QUrl> sources,
                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    static JobHandlePointer deletesHandle(const quint64 windowId, const QList<QUrl> sources,
                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    static JobHandlePointer copyHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                       const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    static JobHandlePointer cutHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    static bool mkdirHandle(const quint64 windowId, const QUrl url, QString *error);

    static bool touchFileHandle(const quint64 windowId, const QUrl url, QString *error, const DFMBASE_NAMESPACE::Global::CreateFileType type);

    static bool renameHandle(const quint64 windowId, const QUrl oldUrl, const QUrl newUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags, QString *error);

    static bool renameFilesHandle(const quint64 windowId, const QList<QUrl> urlList, const QPair<QString, QString> replacePair, bool flg);

    static bool renameFilesHandleAddText(const quint64 windowId, const QList<QUrl> urlList, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> replacePair);

    static QList<QUrl> transUrlsToLocal(const QList<QUrl> &urls);
};
}
#endif   // FILEMANIPULATION_H
