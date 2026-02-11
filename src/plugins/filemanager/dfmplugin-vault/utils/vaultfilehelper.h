// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTFILEHELPER_H
#define VAULTFILEHELPER_H

#include "dfmplugin_vault_global.h"

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>

#include <QObject>
#include <QFileDevice>

namespace dfmplugin_vault {
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
    bool makeDir(const quint64 windowId,
                 const QUrl url, const QUrl &targetUrl,
                 const QVariant custom, DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);
    bool touchFile(const quint64 windowId,
                   const QUrl url, const QUrl &targetUrl,
                   const DFMGLOBAL_NAMESPACE::CreateFileType type, const QString &suffix,
                   const QVariant &custom, DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback,
                   QString *error);
    bool touchCustomFile(const quint64 windowId, const QUrl url, const QUrl &targetUrl,
                         const QUrl tempUrl,
                         const QString &suffix, const QVariant &custom,
                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback, QString *error);
    bool writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                              const QList<QUrl> urls);
    bool renameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> replacePair, bool flg);
    bool renameFilesAddText(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> replacePair);
    bool checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool setPermision(const quint64 windowId,
                      const QUrl url,
                      const QFileDevice::Permissions permissions,
                      bool *ok,
                      QString *error);
    void callBackFunction(const DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus args);
    void handleFinishedNotify(const JobInfoPointer &jobInfo);

private:
    explicit VaultFileHelper(QObject *parent = nullptr);
    QList<QUrl> transUrlsToLocal(const QList<QUrl> &urls);
};
}

#endif   // VAULTFILEHELPER_H
