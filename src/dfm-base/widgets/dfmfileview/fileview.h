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

#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "dfm-base/dfm_base_global.h"

#include <DListView>

DWIDGET_USE_NAMESPACE
DFMBASE_BEGIN_NAMESPACE
class FileViewModel;
class FileViewPrivate;
class BaseItemDelegate;
class FileView : public DListView
{
    Q_OBJECT
    friend class FileViewPrivate;
    QSharedPointer<FileViewPrivate> d;

public:
    explicit FileView(QWidget *parent = nullptr);
    virtual void setViewMode(QListView::ViewMode mode);
    virtual void setDelegate(QListView::ViewMode mode, BaseItemDelegate *view);
    virtual void setRootUrl(const QUrl &url);
    virtual QUrl rootUrl();
    virtual FileViewModel *model();
    virtual void setModel(QAbstractItemModel *model) override;

    int getColumnWidth(const int &column) const;
    int getHeaderViewWidth() const;

public slots:
    void onHeaderViewMouseReleased();
    void onHeaderSectionResized(int logicalIndex, int oldSize, int newSize);
    void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void onClicked(const QModelIndex &index);

protected:
    void resizeEvent(QResizeEvent *event) override;
    QModelIndexList selectedIndexes() const override;

Q_SIGNALS:
    void urlClicked(const QUrl &url);
    void fileClicked(const QUrl &url);
    void dirClicked(const QUrl &url);

private:
    void initializeModel();
    void initializeDelegate();
};
DFMBASE_END_NAMESPACE

#endif   // FILEVIEW_H
