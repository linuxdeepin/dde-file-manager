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
#ifndef FILEOPERATOR_H
#define FILEOPERATOR_H

#include "ddplugin_organizer_global.h"
#include "mode/collectiondataprovider.h"

#include "dfm-base/dfm_global_defines.h"

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

    void dropFilesToCollection(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls, const QString &key, const int index);
    void dropFilesToCanvas(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls);
    void dropToTrash(const QList<QUrl> &urls);
    void dropToApp(const QList<QUrl> &urls, const QString &app);

    QHash<QUrl, QUrl> renameFileData() const;
    void removeRenameFileData(const QUrl &oldUrl);
    void clearRenameFileData();
public:
    void callBackFunction(const DFMBASE_NAMESPACE::Global::CallbackArgus args);

protected:
    explicit FileOperator(QObject *parent = nullptr);

private:
    QSharedPointer<FileOperatorPrivate> d = nullptr;
};

#define FileOperatorIns FileOperator::instance()

}

#endif // FILEOPERATOR_H
