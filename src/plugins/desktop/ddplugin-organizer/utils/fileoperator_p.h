/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

    DFMGLOBAL_NAMESPACE::OperatorCallback callBack;

    QPair<QString, QPair<int, QPoint>> touchFileData;
    QHash<QUrl, QUrl> renameFileData;
};

}

Q_DECLARE_METATYPE(ddplugin_organizer::FileOperatorPrivate::CallBackFunc)

#endif // FILEOPERATOR_P_H
