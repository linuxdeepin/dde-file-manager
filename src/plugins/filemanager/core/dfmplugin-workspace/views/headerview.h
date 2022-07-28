/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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

#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include "dfmplugin_workspace_global.h"

#include "dfm-base/dfm_global_defines.h"

#include <QHeaderView>

namespace dfmplugin_workspace {

class FileView;
class FileSortFilterProxyModel;
class HeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit HeaderView(Qt::Orientation orientation, FileView *parent = nullptr);
    QSize sizeHint() const override;
    using QHeaderView::updateGeometries;

    int sectionsTotalWidth() const;
    void updateColumnWidth();
    void doFileNameColumnResize(const int totalWidth);

public slots:
    void onActionClicked(const int column, QAction *action);

protected:
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *e) override;

Q_SIGNALS:
    void mouseReleased();
    void viewResized();
    void hiddenSectionChanged(const QString &roleName, const bool checked);

private:
    FileSortFilterProxyModel *proxyModel() const;

    FileView *view { nullptr };
    int firstVisibleColumn = -1;
    int lastVisibleColumn = -1;
    bool isChangeCursorState = false;
};

}

#endif   // DFMHEADERVIEW_H
