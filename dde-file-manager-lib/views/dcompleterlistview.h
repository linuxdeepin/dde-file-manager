/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#ifndef DCOMPLETERLISTVIEW_H
#define DCOMPLETERLISTVIEW_H

#include <QListView>
#include <QTimer>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DCompleterListView : public QListView
{
    Q_OBJECT

public:
    explicit DCompleterListView(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *e) override;

    void showMe();
public slots:
    void hideMe();
protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

signals:
    void listCurrentChanged(const QModelIndex &current);
    void listSelectionChanged(const QItemSelection &selected);

};

DFM_END_NAMESPACE

#endif // DCOMPLETERLISTVIEW_H
