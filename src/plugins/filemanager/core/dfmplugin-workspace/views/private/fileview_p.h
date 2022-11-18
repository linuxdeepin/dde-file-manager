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
#ifndef FILEVIEW_P_H
#define FILEVIEW_P_H

#include "views/fileview.h"
#include "utils/workspacehelper.h"

#include "dfm-base/dfm_global_defines.h"

#include <DAnchors>

#include <QObject>
#include <QUrl>
#include <QLabel>

namespace GlobalPrivate {
inline constexpr int kListViewMinimumWidth { 80 };
inline constexpr int kListViewDefaultWidth { 120 };
}   // namespace GlobalPrivate

namespace dfmplugin_workspace {

class ShortcutHelper;
class FileViewHelper;
class FileViewStatusBar;
class HeaderView;
class BaseItemDelegate;
class FileSortFilterProxyModel;
class FileViewPrivate
{
    friend class FileView;
    FileView *const q;

    QAtomicInteger<bool> allowedAdjustColumnSize { true };
    QAtomicInteger<bool> adjustFileNameColumn { true };

    QHash<int, BaseItemDelegate *> delegates;
    FileViewStatusBar *statusBar { nullptr };
    HeaderView *headerView { nullptr };
    DAnchors<QLabel> contentLabel { nullptr };
    FileSortFilterProxyModel *proxyModel { nullptr };
    QTimer *updateStatusBarTimer { nullptr };
    QTimer *updateActiveIndexTimer { nullptr };
    QUrl url;

    ShortcutHelper *shortcutHelper { nullptr };
    DragDropHelper *dragDropHelper { nullptr };
    ViewDrawHelper *viewDrawHelper { nullptr };
    SelectHelper *selectHelper { nullptr };
    FileViewMenuHelper *viewMenuHelper { nullptr };
    FileViewHelper *fileViewHelper { nullptr };

    QList<FileView::SelectionMode> enabledSelectionModes;
    DFMBASE_NAMESPACE::Global::ViewMode currentViewMode = DFMBASE_NAMESPACE::Global::ViewMode::kIconMode;
    int currentIconSizeLevel = 1;
    DFMGLOBAL_NAMESPACE::ItemRoles currentSortRole = DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole;
    Qt::SortOrder currentSortOrder = Qt::SortOrder::AscendingOrder;

    FileView::RandeIndex visibleIndexRande;

    bool isAlwaysOpenInCurrentWindow { false };
    // move cursor later selecte index when pressed key shift
    QModelIndex lastCursorIndex;
    QModelIndex lastMousePressedIndex;

    int horizontalOffset { 0 };
    int cachedViewWidth { -1 };
    bool isShowViewSelectBox { false };

    QList<QUrl> preSelectionUrls;

    QList<DFMGLOBAL_NAMESPACE::ItemRoles> columnRoles;
    QMap<QString, bool> columnForRoleHiddenMap;

    explicit FileViewPrivate(FileView *qq);
    int iconModeColumnCount(int itemWidth = 0) const;
    QUrl modelIndexUrl(const QModelIndex &index) const;

    void initIconModeView();
    void initListModeView();

    QModelIndexList selectedDraggableIndexes();

    void initContentLabel();
    void updateHorizontalScrollBarPosition();
    void pureResizeEvent(QResizeEvent *event);

    void loadViewMode(const QUrl &url);
    QVariant fileViewStateValue(const QUrl &url, const QString &key, const QVariant &defalutValue);
};

}

#endif   // FILEVIEW_P_H
