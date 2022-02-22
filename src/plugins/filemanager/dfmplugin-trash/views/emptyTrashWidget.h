/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#ifndef EMPTYTRASHWIDGET_H
#define EMPTYTRASHWIDGET_H

#include "dfmplugin_trash_global.h"
#include <QFrame>

DPTRASH_BEGIN_NAMESPACE

class EmptyTrashWidget : public QFrame
{
    Q_OBJECT

public:
    explicit EmptyTrashWidget(QWidget *parrent = nullptr);
signals:
    void emptyTrash();
};

DPTRASH_END_NAMESPACE

#endif   // EMPTYTRASHWIDGET_H
