// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OLDFILEPROPERTY_H
#define OLDFILEPROPERTY_H

#include <QObject>

class OldFileProperty : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "file_property")
    Q_PROPERTY(QString file_name READ getFilePath WRITE setFilePath)
    Q_PROPERTY(QString tag_1 READ getTag1 WRITE setTag1)
    Q_PROPERTY(QString tag_2 READ getTag2 WRITE setTag2)
    Q_PROPERTY(QString tag_3 READ getTag3 WRITE setTag3)
public:
    explicit OldFileProperty(QObject *parent = nullptr);

    QString getFilePath() const;
    void setFilePath(const QString &value);

    QString getTag1() const;
    void setTag1(const QString &value);

    QString getTag2() const;
    void setTag2(const QString &value);

    QString getTag3() const;
    void setTag3(const QString &value);

private:
    QString file_name{};
    QString tag_1{};
    QString tag_2{};
    QString tag_3{};
};

#endif // OLDFILEPROPERTY_H
