/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
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
#ifndef VAULTFILEHELPER_H
#define VAULTFILEHELPER_H

#include "dfmplugin_vault_global.h"

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/dfm_global_defines.h"

#include <QObject>

DPVAULT_BEGIN_NAMESPACE

class VaultFileHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultFileHelper)
public:
    static VaultFileHelper *instance();
    inline static QString scheme()
    {
        return "dfmvault";
    }
    bool cutFile(const quint64 windowId, const QList<QUrl> sources,
                 const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool copyFile(const quint64 windowId, const QList<QUrl> sources,
                  const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool moveToTrash(const quint64 windowId, const QList<QUrl> sources,
                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool deleteFile(const quint64 windowId, const QList<QUrl> sources,
                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool openFileInPlugin(quint64 windowId, const QList<QUrl> urls);
    bool renameFile(const quint64 windowId, const QUrl oldUrl, const QUrl newUrl,
                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool makeDir(const quint64 windowId, const QUrl url);
    bool touchFile(const quint64 windowId, const QUrl url, const DFMGLOBAL_NAMESPACE::CreateFileType type, QString *error);
    bool touchCustomFile(const quint64 windowId, const QUrl url, const QUrl tempUrl, QString *error);
    bool writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                              const QList<QUrl> urls);
    bool renameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> replacePair, bool flg);
    bool renameFilesAddText(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> replacePair);
    bool checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool openFileByApp(const quint64 windowId, const QList<QUrl> urls, const QList<QString> apps);

private:
    QList<QUrl> transUrlsToLocal(const QList<QUrl> &urls);

private:
    explicit VaultFileHelper(QObject *parent = nullptr);
};
DPVAULT_END_NAMESPACE

#endif   // VAULTFILEHELPER_H
