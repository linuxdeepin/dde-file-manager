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
#ifndef NORMALIZEDMODEBROKER_H
#define NORMALIZEDMODEBROKER_H

#include "broker/organizerbroker.h"

namespace ddplugin_organizer {
class NormalizedMode;
class CollectionView;
class NormalizedModeBroker : public OrganizerBroker
{
    Q_OBJECT
public:
    explicit NormalizedModeBroker(NormalizedMode *parent = nullptr);
public slots:
    QString gridPoint(const QUrl &item, QPoint *point) override;
    QRect visualRect(const QString &id, const QUrl &item) override;
    QAbstractItemView *view(const QString &id) override;
    QRect iconRect(const QString &id, QRect vrect) override;
private:
    NormalizedMode *mode;
};

}

#endif // NORMALIZEDMODEBROKER_H
