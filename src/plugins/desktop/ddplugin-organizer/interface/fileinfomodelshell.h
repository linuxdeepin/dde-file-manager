// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFOMODELSHELL_H
#define FILEINFOMODELSHELL_H

#include "ddplugin_organizer_global.h"

#include <dfm-base/file/local/syncfileinfo.h>

#include <QObject>

class QAbstractItemModel;

namespace ddplugin_organizer {

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
    FileInfoPointer fileInfo(const QModelIndex &index) const;
    QUrl fileUrl(const QModelIndex &index) const;
    QList<QUrl> files() const;
    void refresh(const QModelIndex &parent);
    int modelState();
signals:
    void dataReplaced(const QUrl &oldUrl, const QUrl &newUrl);
public slots:
protected:
    mutable QAbstractItemModel *model = nullptr;
};

}

#endif   // FILEINFOMODELSHELL_H
