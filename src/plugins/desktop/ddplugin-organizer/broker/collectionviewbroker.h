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
#ifndef COLLECTIONVIEWBROKER_H
#define COLLECTIONVIEWBROKER_H

#include <QObject>
#include <QRect>

namespace ddplugin_organizer {

class CollectionView;
class CollectionViewBroker : public QObject
{
    Q_OBJECT
public:
    explicit CollectionViewBroker(CollectionView *parent = nullptr);
    inline CollectionView *getView() const {return view;}
    void setView(CollectionView *v);
    bool gridPoint(const QUrl &file, QPoint &pos) const;
    QRect visualRect(const QUrl &file) const;
signals:

public slots:
private:
    CollectionView *view = nullptr;
};

}

#endif // COLLECTIONVIEWBROKER_H
