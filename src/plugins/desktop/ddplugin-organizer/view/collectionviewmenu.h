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
#ifndef COLLECTIONVIEWMENU_H
#define COLLECTIONVIEWMENU_H

#include <QObject>

namespace ddplugin_organizer {
class CollectionView;
class CollectionViewMenu : public QObject
{
    Q_OBJECT
public:
    explicit CollectionViewMenu(CollectionView *parent);
    static bool disableMenu();
    void emptyAreaMenu();
    void normalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos);
signals:

public slots:
protected:
    QWidget *getCanvasView();
private:
    CollectionView *view;
};
}

#endif // COLLECTIONVIEWMENU_H
