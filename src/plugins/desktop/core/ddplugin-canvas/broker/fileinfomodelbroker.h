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

namespace ddplugin_canvas {
class FileInfoModel;
class FileInfoModelBroker : public QObject
{
    Q_OBJECT
public:
    explicit FileInfoModelBroker(FileInfoModel *model, QObject *parent = nullptr);
    ~FileInfoModelBroker();
    bool init();
signals:

public slots:
    QUrl rootUrl();
    QModelIndex rootIndex();
    QModelIndex urlIndex(const QUrl &url);
    QUrl indexUrl(const QModelIndex &index);
    QList<QUrl> files();
    DFMLocalFileInfoPointer fileInfo(const QModelIndex &index);
    void refresh(const QModelIndex &parent);
    int modelState();
    void updateFile(const QUrl &url);
private slots:
    void onDataReplaced(const QUrl &oldUrl, const QUrl &newUrl);

private:
    FileInfoModel *model = nullptr;
};

}

#endif   // FILEINFOMODELBROKER_H
