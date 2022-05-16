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
#ifndef FILEINFOMODELBROKER_H
#define FILEINFOMODELBROKER_H

#include "ddplugin_canvas_global.h"

#include <file/local/localfileinfo.h>

#include <QObject>

DDP_CANVAS_BEGIN_NAMESPACE
class FileInfoModel;
class FileInfoModelBrokerPrivate;
class FileInfoModelBroker : public QObject
{
    Q_OBJECT
    friend class FileInfoModelBrokerPrivate;
public:
    explicit FileInfoModelBroker(FileInfoModel *model, QObject *parent = nullptr);
    bool init();
signals:

public slots:
    void rootUrl(QUrl *url);
    void rootIndex(QModelIndex *root);
    void urlIndex(const QUrl &url, QModelIndex *index);
    void indexUrl(const QModelIndex &index, QUrl *url);
    void files(QList<QUrl> *urls);
    void fileInfo(const QModelIndex &index, DFMLocalFileInfoPointer *file);
    void refresh(const QModelIndex &parent);
private:
    FileInfoModelBrokerPrivate *d;
};

DDP_CANVAS_END_NAMESPACE

#endif // FILEINFOMODELBROKER_H
