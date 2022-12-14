/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu <zhuangshu@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef DEFAULTITEMMANAGER_H
#define DEFAULTITEMMANAGER_H

#include "units/bookmarkupgradeunit.h"
#include <QObject>
namespace dfm_upgrade {

class DefaultItemManagerPrivate;
class DefaultItemManager : public QObject
{
    Q_OBJECT
    friend class DefaultItemManagerPrivate;
    DefaultItemManagerPrivate *const d = nullptr;

public:
    static DefaultItemManager *instance();
    void initDefaultItems();
    QList<BookmarkData> defaultItemInitOrder();

private:
    explicit DefaultItemManager(QObject *parent = nullptr);
};

}

#endif   // DEFAULTITEMMANAGER_H
