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
#ifndef COMPLETERVIEWMODEL_H
#define COMPLETERVIEWMODEL_H

#include "dfmplugin_titlebar_global.h"

#include <DStandardItem>

#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE

namespace dfmplugin_titlebar {

class CompleterViewModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit CompleterViewModel(QObject *parent = nullptr);
    ~CompleterViewModel();

    void setStringList(const QStringList &list);
    void removeAll();
};
}

#endif   // COMPLETERVIEWMODEL_H
