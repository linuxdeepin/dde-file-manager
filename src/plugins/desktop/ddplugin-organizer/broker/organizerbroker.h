/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef ORGANIZERBROKER_H
#define ORGANIZERBROKER_H

#include <QObject>
#include <QPoint>
#include <QRect>

class QAbstractItemView;
namespace ddplugin_organizer {

class OrganizerBroker : public QObject
{
    Q_OBJECT
public:
    explicit OrganizerBroker(QObject *parent = nullptr);
    ~OrganizerBroker() override;
    virtual bool init();

signals:

public slots:
    virtual QString gridPoint(const QUrl &item, QPoint *point) = 0;
    virtual QRect visualRect(const QString &id, const QUrl &item) = 0;
    virtual QAbstractItemView * view(const QString &id) = 0;
    virtual QRect iconRect(const QString &id, QRect vrect) = 0;
};
}

#endif // ORGANIZERBROKER_H
