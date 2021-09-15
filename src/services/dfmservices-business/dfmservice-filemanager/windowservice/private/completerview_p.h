/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#ifndef COMPLETERVIEW_P_H
#define COMPLETERVIEW_P_H

#include "dfm_filemanager_service_global.h"

#include <QObject>
#include <QCompleter>
#include <QListView>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFileSystemModel>

DSB_FM_BEGIN_NAMESPACE

class CompleterViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CompleterViewDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

class CompleterView;
class CompleterViewPrivate : public QObject
{
    Q_OBJECT
    friend class CompleterView;
    CompleterView *const q;

    QCompleter completer;
    QStringListModel model;
    CompleterViewDelegate delegate;

    explicit CompleterViewPrivate(CompleterView *qq);
};

DSB_FM_END_NAMESPACE

#endif // COMPLETERVIEW_P_H
