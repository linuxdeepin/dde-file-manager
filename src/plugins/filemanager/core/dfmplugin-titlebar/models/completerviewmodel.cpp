/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "completerviewmodel.h"

#include <QAbstractItemView>

using namespace dfmplugin_titlebar;

CompleterViewModel::CompleterViewModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

CompleterViewModel::~CompleterViewModel()
{
}

void CompleterViewModel::setStringList(const QStringList &list)
{
    removeAll();
    for (const auto &str : list) {
        if (str.isEmpty())
            continue;

        QStandardItem *item = new QStandardItem(str);
        appendRow(item);
    }
}

void CompleterViewModel::removeAll()
{
    removeRows(0, rowCount());
}
