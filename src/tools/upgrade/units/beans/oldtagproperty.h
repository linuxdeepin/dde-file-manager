// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OLDTAGPROPERTY_H
#define OLDTAGPROPERTY_H

#include <QObject>

class OldTagProperty : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "tag_property")
    Q_PROPERTY(int tag_index READ getTagIndex WRITE setTagIndex)
    Q_PROPERTY(QString tag_name READ getTagName WRITE setTagName)
    Q_PROPERTY(QString tag_color READ getTagColor WRITE setTagColor)
public:
    explicit OldTagProperty(QObject *parent = nullptr);

    int getTagIndex() const;
    void setTagIndex(int value);

    QString getTagName() const;
    void setTagName(const QString &value);

    QString getTagColor() const;
    void setTagColor(const QString &value);

private:
    int tag_index {};
    QString tag_name {};
    QString tag_color {};
};

#endif // OLDTAGPROPERTY_H
