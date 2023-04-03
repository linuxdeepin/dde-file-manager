// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace dfmbase {

struct ShortcutItem
{
    QString name;
    QString value;
    ShortcutItem(QString n, QString v)
        : name(n), value(v) {}
};

struct ShortcutGroup
{
    QString groupName;
    QList<ShortcutItem> groupItems;
};

class Shortcut : public QObject
{
    Q_OBJECT
public:
    explicit Shortcut(QObject *parent = nullptr);
    QString toStr();

Q_SIGNALS:

public Q_SLOTS:

private:
    QJsonObject shortcutObj;
    QList<ShortcutGroup> shortcutGroups;
};

}
#endif   // SHORTCUT_H
