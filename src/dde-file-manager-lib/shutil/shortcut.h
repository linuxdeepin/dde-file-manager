/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
struct ShortcutItem{
    QString name;
    QString value;
    ShortcutItem(QString n,QString v):name(n),value(v){}
};

struct ShortcutGroup{
    QString groupName;
    QList<ShortcutItem> groupItems;
};

class Shortcut : public QObject
{
    Q_OBJECT
public:
    explicit Shortcut(QObject *parent = 0);
    QString toStr();

signals:

public slots:
private:
    QJsonObject m_shortcutObj;
    QList<ShortcutGroup> m_shortcutGroups;
};

#endif // SHORTCUT_H
