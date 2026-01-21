// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILETAGINFO_H
#define TRASHFILETAGINFO_H

#include "daemonplugin_tag_global.h"

#include <QObject>

DAEMONPTAG_BEGIN_NAMESPACE

class TrashFileTagInfo : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("TableName", "trash_file_tags")
    Q_PROPERTY(int trashIndex READ getTrashIndex WRITE setTrashIndex)
    Q_PROPERTY(QString originalPath READ getOriginalPath WRITE setOriginalPath)
    Q_PROPERTY(qint64 fileInode READ getFileInode WRITE setFileInode)
    Q_PROPERTY(QString tagNames READ getTagNames WRITE setTagNames)
    Q_PROPERTY(qint64 deleteTime READ getDeleteTime WRITE setDeleteTime)
    Q_PROPERTY(QString future READ getFuture WRITE setFuture)

public:
    explicit TrashFileTagInfo(QObject *parent = nullptr);

    int getTrashIndex() const;
    void setTrashIndex(int value);

    QString getOriginalPath() const;
    void setOriginalPath(const QString &value);

    qint64 getFileInode() const;
    void setFileInode(qint64 value);

    QString getTagNames() const;
    void setTagNames(const QString &value);

    qint64 getDeleteTime() const;
    void setDeleteTime(qint64 value);

    QString getFuture() const;
    void setFuture(const QString &value);

private:
    int trashIndex {};
    QString originalPath {};
    qint64 fileInode {};
    QString tagNames {};
    qint64 deleteTime {};
    QString future {};
};

DAEMONPTAG_END_NAMESPACE

#endif   // TRASHFILETAGINFO_H
