// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATOR_P_H
#define FILEOPERATOR_P_H

#include "fileoperator.h"

#include <QObject>
#include <QPointer>

namespace ddplugin_organizer {

class FileOperatorPrivate : public QObject
{
    Q_OBJECT
public:
    enum CallBackFunc {
        kCallBackTouchFile,
        kCallBackTouchFolder,
        kCallBackCopyFiles,
        kCallBackCutFiles,
        kCallBackPasteFiles,
        kCallBackOpenFiles,
        kCallBackRenameFiles,
        kCallBackOpenFilesByApp,
        kCallBackMoveToTrash,
        kCallBackDeleteFiles
    };

    explicit FileOperatorPrivate(FileOperator *qq);

    void callBackTouchFile(const QUrl &target, const QVariantMap &customData);
    void callBackPasteFiles(const JobInfoPointer info);
    void callBackRenameFiles(const QList<QUrl> &sources, const QList<QUrl> &targets, const CollectionView *view);

    QList<QUrl> getSelectedUrls(const CollectionView *view) const;
    static void filterDesktopFile(QList<QUrl> &urls);

public:
    FileOperator *q = nullptr;
    QPointer<CollectionDataProvider> provider = nullptr;

    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callBack;

    QPair<QString, QPair<int, QPoint>> touchFileData;
    QHash<QUrl, QUrl> renameFileData;
};

}

Q_DECLARE_METATYPE(ddplugin_organizer::FileOperatorPrivate::CallBackFunc)

#endif   // FILEOPERATOR_P_H
