// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILETAGINFO_H
#define FILETAGINFO_H

#include <QObject>

namespace dfm_upgrade {
class FileTagInfo : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("TableName", "file_tags")
    Q_PROPERTY(int fileIndex READ getFileIndex WRITE setFileIndex)
    Q_PROPERTY(QString filePath READ getFilePath WRITE setFilePath)
    Q_PROPERTY(QString tagName READ getTagName WRITE setTagName)
    Q_PROPERTY(int tagOrder READ getTagOrder WRITE setTagOrder)
    Q_PROPERTY(QString future READ getFuture WRITE setFuture)

public:
    explicit FileTagInfo(QObject *parent = nullptr);

    int getFileIndex() const;
    void setFileIndex(int value);

    QString getFilePath() const;
    void setFilePath(const QString &value);

    QString getTagName() const;
    void setTagName(const QString &value);

    int getTagOrder() const;
    void setTagOrder(int value);

    QString getFuture() const;
    void setFuture(const QString &value);

private:
    int fileIndex {};
    QString filePath {};
    QString tagName {};
    int tagOrder {};
    QString future {};
};
} // dfm_upgrade

#endif   // FILETAGINFO_H
