// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATOR_H
#define FILEOPERATOR_H

#include "ddplugin_organizer_global.h"
#include "mode/collectiondataprovider.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QSharedPointer>

namespace ddplugin_organizer {

class CollectionView;
class FileOperatorPrivate;
class FileOperator : public QObject
{
    Q_OBJECT
    friend class FileOperatorPrivate;

public:
    ~FileOperator();
    static FileOperator *instance();
    void setDataProvider(CollectionDataProvider *provider);

    void copyFiles(const CollectionView *view);
    void cutFiles(const CollectionView *view);
    void pasteFiles(const CollectionView *view);
    void pasteFiles(const CollectionView *view, const QPoint pos);
    void openFiles(const CollectionView *view);
    void openFiles(const CollectionView *view, const QList<QUrl> &urls);
    Q_INVOKABLE void renameFile(int wid, const QUrl &oldUrl, const QUrl &newUrl);
    void renameFiles(const CollectionView *view, const QList<QUrl> &urls, const QPair<QString, QString> &pair, const bool replace);
    void renameFiles(const CollectionView *view, const QList<QUrl> &urls, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair);
    void moveToTrash(const CollectionView *view);
    void deleteFiles(const CollectionView *view);
    void undoFiles(const CollectionView *view);
    void previewFiles(const CollectionView *view);
    void showFilesProperty(const CollectionView *view);
    void dropFilesToCollection(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls, const QString &key, const int index);
    void dropFilesToCanvas(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls);
    void dropToTrash(const QList<QUrl> &urls);
    void dropToApp(const QList<QUrl> &urls, const QString &app);

    QHash<QUrl, QUrl> renameFileData() const;
    void removeRenameFileData(const QUrl &oldUrl);
    void clearRenameFileData();

public:
    void callBackFunction(const DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus args);

protected:
    explicit FileOperator(QObject *parent = nullptr);

private:
    QSharedPointer<FileOperatorPrivate> d = nullptr;
};

#define FileOperatorIns FileOperator::instance()

}

#endif   // FILEOPERATOR_H
