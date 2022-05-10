/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef FILEINFOMODELSHELL_H
#define FILEINFOMODELSHELL_H

#include "ddplugin_organizer_global.h"

#include <file/local/localfileinfo.h>

#include <QObject>

class QAbstractItemModel;

DDP_ORGANIZER_BEGIN_NAMESPACE

class CanvasInterface;
class FileInfoModelShell : public QObject
{
    Q_OBJECT
public:
    explicit FileInfoModelShell(QObject *parent = nullptr);
    ~FileInfoModelShell();
    bool initialize();
    QAbstractItemModel *sourceModel() const;
    QUrl rootUrl() const;
    QModelIndex rootIndex() const;
    QModelIndex index(const QUrl &url, int column = 0) const;
    DFMLocalFileInfoPointer fileInfo(const QModelIndex &index) const;
    QUrl fileUrl(const QModelIndex &index) const;
    QList<QUrl> files() const;
    void refresh(const QModelIndex &parent);
signals:
    void dataReplaced(const QUrl &oldUrl, const QUrl &newUrl);
public slots:
protected:
    mutable QAbstractItemModel *model = nullptr;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // FILEINFOMODELSHELL_H
