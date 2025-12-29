// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATIONSEVENTRECEIVER_H
#define FILEOPERATIONSEVENTRECEIVER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationsservice.h"
#include "fileoperations/filecopymovejob.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/dialogmanager.h>

#include <QObject>
#include <QPointer>
#include <QFileDevice>

#include <memory>

Q_DECLARE_METATYPE(QFileDevice::Permissions)

namespace dfmplugin_fileoperations {
class FileCopyMoveJob;
class FileOperationsEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileOperationsEventReceiver)

public:
    enum class DoDeleteErrorType {
        kNoErrror,
        kSourceEmpty,
        kNullPtr,
    };
    static FileOperationsEventReceiver *instance();

public slots:
    void handleOperationCopy(const quint64 windowId,
                             const QList<QUrl> sources,
                             const QUrl target,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);
    void handleOperationCopy(const quint64 windowId,
                             const QList<QUrl> sources,
                             const QUrl target,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle,
                             const QVariant custom,
                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    void handleOperationCut(const quint64 windowId,
                            const QList<QUrl> sources,
                            const QUrl target,
                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                            DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);

    void handleOperationCut(const quint64 windowId,
                            const QList<QUrl> sources,
                            const QUrl target,
                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                            DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle,
                            const QVariant custom,
                            DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    void handleOperationDeletes(const quint64 windowId,
                                const QList<QUrl> sources,
                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);
    void handleOperationDeletes(const quint64 windowId,
                                const QList<QUrl> sources,
                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle,
                                const QVariant custom,
                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    bool handleOperationOpenFiles(const quint64 windowId, const QList<QUrl> urls);
    bool handleOperationOpenFiles(const quint64 windowId, const QList<QUrl> urls, bool *ok);
    void handleOperationOpenFiles(const quint64 windowId,
                                  const QList<QUrl> urls,
                                  const QVariant custom,
                                  DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    bool handleOperationOpenFilesByApp(const quint64 windowId,
                                       const QList<QUrl> urls,
                                       const QList<QString> apps);
    void handleOperationOpenFilesByApp(const quint64 windowId,
                                       const QList<QUrl> urls,
                                       const QList<QString> apps,
                                       const QVariant custom,
                                       DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    bool handleOperationRenameFile(const quint64 windowId,
                                   const QUrl oldUrl,
                                   const QUrl newUrl,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags);
    void handleOperationRenameFile(const quint64 windowId,
                                   const QUrl oldUrl,
                                   const QUrl newUrl,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                   const QVariant custom,
                                   DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    bool handleOperationRenameFiles(const quint64 windowId,
                                    const QList<QUrl> urls,
                                    const QPair<QString, QString> pair,
                                    const bool replace);
    void handleOperationRenameFiles(const quint64 windowId,
                                    const QList<QUrl> urls,
                                    const QPair<QString, QString> pair,
                                    const bool replace,
                                    const QVariant custom,
                                    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);
    bool handleOperationRenameFiles(const quint64 windowId,
                                    const QList<QUrl> urls,
                                    const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair);
    void handleOperationRenameFiles(const quint64 windowId,
                                    const QList<QUrl> urls,
                                    const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair,
                                    const QVariant custom,
                                    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    bool handleOperationMkdir(const quint64 windowId,
                              const QUrl url);
    void handleOperationMkdir(const quint64 windowId,
                              const QUrl url,
                              const QVariant custom,
                              DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    QString handleOperationTouchFile(const quint64 windowId,
                                     const QUrl url,
                                     const DFMBASE_NAMESPACE::Global::CreateFileType fileType,
                                     const QString suffix);
    QString handleOperationTouchFile(const quint64 windowId,
                                     const QUrl url,
                                     const QUrl tempUrl,
                                     const QString suffix);
    void handleOperationTouchFile(const quint64 windowId,
                                  const QUrl url,
                                  const DFMBASE_NAMESPACE::Global::CreateFileType fileType,
                                  const QString suffix,
                                  const QVariant custom,
                                  DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callbackImmediately);
    void handleOperationTouchFile(const quint64 windowId,
                                  const QUrl url,
                                  const QUrl tempUrl,
                                  const QString suffix,
                                  const QVariant custom,
                                  DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callbackImmediately);

    bool handleOperationLinkFile(const quint64 windowId,
                                 const QUrl url,
                                 const QUrl link,
                                 const bool force,
                                 const bool silence);
    void handleOperationLinkFile(const quint64 windowId,
                                 const QUrl url,
                                 const QUrl link,
                                 const bool force,
                                 const bool silence,
                                 const QVariant custom,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    bool handleOperationSetPermission(const quint64 windowId,
                                      const QUrl url,
                                      const QFileDevice::Permissions permissions);
    void handleOperationSetPermission(const quint64 windowId,
                                      const QUrl url,
                                      const QFileDevice::Permissions permissions,
                                      const QVariant custom,
                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callBack);

    bool handleOperationWriteToClipboard(const quint64 windowId,
                                         const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                         const QList<QUrl> urls);
    bool handleOperationWriteDataToClipboard(const quint64 windowId,
                                             QMimeData *data);
    bool handleOperationOpenInTerminal(const quint64 windowId,
                                       const QList<QUrl> urls);
    void handleOperationSaveOperations(const QVariantMap values);
    void handleOperationCleanSaveOperationsStack();
    void handleOperationRevocation(const quint64 windowId,
                                   DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);

    bool handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls);
    void handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls,
                                  const QVariant custom, DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback);

    bool handleShortCut(quint64, const QList<QUrl> &urls, const QUrl &rootUrl);
    bool handleShortCutPaste(quint64, const QList<QUrl> &, const QUrl &target);
    void handleOperationSaveRedoOperations(const QVariantMap &values);
    void handleOperationCleanByUrls(const QList<QUrl> &urls);
    void handleRecoveryOperationRedoRecovery(const quint64 windowId,
                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);
    void handleSaveRedoOpt(const QString &token, const qint64 fileSize);
    void handleOperationUndoDeletes(const quint64 windowId,
                                    const QList<QUrl> &sources,
                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                    const QVariantMap &op);
    void handleOperationUndoCut(const quint64 windowId,
                                const QList<QUrl> &sources,
                                const QUrl target,
                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                const QVariantMap &op);
    void handleOperationFilesPreview(const quint64 windowId,
                                     const QList<QUrl> &selectUrls,
                                     const QList<QUrl> &dirUrls);
    bool handleIsSubFile(const QUrl &parent, const QUrl &sub);
    void handleCopyFilePath(const QList<QUrl> &urlList);

private:
    enum class RenameTypes {
        kBatchRepalce,
        kBatchCustom,
        kBatchAppend
    };

    explicit FileOperationsEventReceiver(QObject *parent = nullptr);
    QString newDocmentName(const QUrl &url,
                           const QString &suffix,
                           const DFMBASE_NAMESPACE::Global::CreateFileType fileType);
    QString newDocmentName(const QUrl &url,
                           const QString &baseName,
                           const QString &suffix);

    bool revocation(const quint64 windowId, const QVariantMap &ret,
                    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);
    bool redo(const quint64 windowId, const QVariantMap &ret,
              DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle);

    bool doRenameFiles(const quint64 windowId, const QList<QUrl> &urls,
                       const QPair<QString, QString> &pair,
                       const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> &pair2,
                       const RenameTypes type,
                       QMap<QUrl, QUrl> &successUrls, QString &errorMsg,
                       const QVariant custom = QVariant(), DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback = nullptr);
    bool doRenameDesktopFile(const quint64 windowId,
                             const QUrl oldUrl,
                             const QUrl newUrl,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool doRenameDesktopFiles(QList<QUrl> &urls,
                              const QPair<QString, QString> pair,
                              QMap<QUrl, QUrl> &needDealUrls,
                              QMap<QUrl, QUrl> &successUrls);

    JobHandlePointer doCopyFile(const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback callbaskHandle);
    JobHandlePointer doCutFile(quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                               const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                               DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                               const bool isInit = true);
    JobHandlePointer doDeleteFile(const quint64 windowId, const QList<QUrl> &sources,
                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                  DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                  const bool isInit,
                                  DoDeleteErrorType &errorType);
    JobHandlePointer doCleanTrash(const quint64 windowId, const QList<QUrl> &sources,
                                  const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                  DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback);
    bool doMkdir(const quint64 windowId, const QUrl &url,
                 const QVariant &custom,
                 DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback,
                 const bool useUrlPath = false);
    QString doTouchFilePremature(const quint64 windowId, const QUrl &url,
                                 const DFMBASE_NAMESPACE::Global::CreateFileType fileType, const QString &suffix,
                                 const QVariant &custom,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callbackImmediately);
    QString doTouchFilePremature(const quint64 windowId, const QUrl &url,
                                 const QUrl &tempUrl, const QString &suffix,
                                 const QVariant &custom,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callbackImmediately);
    bool doTouchFilePractically(const quint64 windowId, const QUrl &url, const QUrl &tempUrl = QUrl());
    QString doTouchFileFromClipboard(const quint64 windowId, const QUrl &url,
                                     const QString &suffix, const QVariant &custom,
                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callbackImmediately);
    void saveFileOperation(const QList<QUrl> &sourcesUrls, const QList<QUrl> &targetUrls,
                           DFMBASE_NAMESPACE::GlobalEventType type,
                           const QList<QUrl> &redoSourcesUrls, const QList<QUrl> &redoTargetUrls,
                           const dfmbase::GlobalEventType redo,
                           const bool isUndo = false, const QUrl &templateUrl = QUrl());
    QUrl determineLinkTarget(const QUrl &sourceUrl, const QUrl &linkUrl, const bool silence, const quint64 windowId);
    bool doOpenInTerminal(const QUrl &url);

private:
    std::unique_ptr<FileCopyMoveJob> copyMoveJob { std::make_unique<FileCopyMoveJob>() };
    DFMBASE_NAMESPACE::DialogManager *dialogManager { nullptr };
    QMap<QString, QVariantMap> undoOpts;
    QMutex undoLock;
};

}

Q_DECLARE_METATYPE(QSharedPointer<QList<QUrl>>)

#endif   // FILEOPERATIONSEVENTRECEIVER_H
