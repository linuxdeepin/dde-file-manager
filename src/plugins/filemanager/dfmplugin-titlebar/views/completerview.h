/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMCOMPLETERVIEW_H
#define DFMCOMPLETERVIEW_H

#include "dfmplugin_titlebar_global.h"

#include <QListView>
#include <QFileSystemModel>

DPTITLEBAR_BEGIN_NAMESPACE
class CompleterView : public QListView
{
    Q_OBJECT
public:
    explicit CompleterView(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *e) override;

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

signals:
    void listCurrentChanged(const QModelIndex &current);
    void listSelectionChanged(const QItemSelection &selected);
};
DPTITLEBAR_END_NAMESPACE

#endif   //DFMCOMPLETERVIEW_H
