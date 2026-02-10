// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OLDFILEPROPERTY_H
#define OLDFILEPROPERTY_H

#include <QObject>

class OldFileProperty : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "tag_with_file")
    Q_PROPERTY(QString file_name READ getFilePath WRITE setFilePath)
    Q_PROPERTY(QString tag_name READ getTag WRITE setTag)

public:
    explicit OldFileProperty(QObject *parent = nullptr);

    QString getFilePath() const;
    void setFilePath(const QString &value);

    QString getTag() const;
    void setTag(const QString &value);

private:
    QString file_name {};
    QString tag_name {};
};

#endif   // OLDFILEPROPERTY_H
