// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFOMODELBROKER_H
#define FILEINFOMODELBROKER_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/file/local/syncfileinfo.h>

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
    DFMSyncFileInfoPointer fileInfo(const QModelIndex &index);
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
