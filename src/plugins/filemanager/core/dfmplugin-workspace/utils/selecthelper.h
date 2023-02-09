/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#ifndef SELECTHELPER_H
#define SELECTHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QModelIndex>
#include <QItemSelectionModel>

QT_BEGIN_HEADER
class QItemSelection;
QT_END_NAMESPACE

namespace dfmplugin_workspace {
class FileView;
class SelectHelper : public QObject
{
    Q_OBJECT

public:
    explicit SelectHelper(FileView *parent);
    QModelIndex getCurrentPressedIndex() const;
    void click(const QModelIndex &index);
    void release();
    void setSelection(const QItemSelection &selection);
    void selection(const QRect &rect, QItemSelectionModel::SelectionFlags flags);
    void select(const QList<QUrl> &urls);
    void select(const QList<QModelIndex> &indexes);

private:
    void caculateSelection(const QRect &rect, QItemSelection *selection);
    void caculateIconViewSelection(const QRect &rect, QItemSelection *selection);
    void caculateListViewSelection(const QRect &rect, QItemSelection *selection);

private:
    FileView *view { nullptr };
    QModelIndex lastPressedIndex;
    QModelIndex currentPressedIndex;
    QItemSelection currentSelection;
};

}
#endif   // SELECTHELPER_H
