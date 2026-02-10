// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGPROPERTY_H
#define TAGPROPERTY_H

#include "daemonplugin_tag_global.h"

#include <QObject>

DAEMONPTAG_BEGIN_NAMESPACE

class TagProperty : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("TableName", "tag_property")
    Q_PROPERTY(int tagIndex READ getTagIndex WRITE setTagIndex)
    Q_PROPERTY(QString tagName READ getTagName WRITE setTagName)
    Q_PROPERTY(QString tagColor READ getTagColor WRITE setTagColor)
    Q_PROPERTY(int ambiguity READ getAmbiguity WRITE setAmbiguity)
    Q_PROPERTY(QString future READ getFuture WRITE setFuture)

public:
    explicit TagProperty(QObject *parent = nullptr);

    int getTagIndex() const;
    void setTagIndex(int value);

    QString getTagName() const;
    void setTagName(const QString &value);

    QString getTagColor() const;
    void setTagColor(const QString &value);

    int getAmbiguity() const;
    void setAmbiguity(int value);

    QString getFuture() const;
    void setFuture(const QString &value);

private:
    int tagIndex {};
    QString tagName {};
    QString tagColor {};
    int ambiguity {};
    QString future {};
};

DAEMONPTAG_END_NAMESPACE

#endif   // TAGPROPERTY_H
