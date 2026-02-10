// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEW_P_H
#define FILEVIEW_P_H

#include "views/fileview.h"
#include "utils/workspacehelper.h"

#include <dfm-base/dfm_global_defines.h>

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
class ViewAnimationHelper;
class FileViewStatusBar;
class HeaderView;
class BaseItemDelegate;
class FileViewPrivate
{
    friend class FileView;
    FileView *const q;

    QAtomicInteger<bool> allowedAdjustColumnSize { true };
    QAtomicInteger<bool> adjustFileNameColumn { true };

    QHash<int, BaseItemDelegate *> delegates;
    FileViewStatusBar *statusBar { nullptr };
    HeaderView *headerView { nullptr };
    int oldHeaderViewLenght { 0 };
    DAnchors<QLabel> contentLabel { nullptr };
    QTimer *updateStatusBarTimer { nullptr };
    QUrl url;

    ShortcutHelper *shortcutHelper { nullptr };
    DragDropHelper *dragDropHelper { nullptr };
    ViewDrawHelper *viewDrawHelper { nullptr };
    SelectHelper *selectHelper { nullptr };
    FileViewMenuHelper *viewMenuHelper { nullptr };
    FileViewHelper *fileViewHelper { nullptr };
    ViewAnimationHelper *animationHelper { nullptr };
    QWidget *headerWidget { nullptr };

    QList<FileView::SelectionMode> enabledSelectionModes;
    DFMBASE_NAMESPACE::Global::ViewMode currentViewMode = DFMBASE_NAMESPACE::Global::ViewMode::kIconMode;
    int currentIconSizeLevel = 5;
    int currentGridDensityLevel = 1;
    int currentListHeightLevel = 1;
    bool isAlwaysOpenInCurrentWindow { false };
    // move cursor later selecte index when pressed key shift
    QModelIndex lastCursorIndex;
    QModelIndex lastMousePressedIndex;
    QModelIndex lastClickedIndex;

    int horizontalOffset { 0 };
    int cachedViewWidth { -1 };
    bool isShowViewSelectBox { false };
    bool isResizeEvent { false };
    int lastContentHeight { 0 };

    QList<QUrl> preSelectionUrls;
    QTimer *preSelectTimer { nullptr };
    QTimer *groupHeaderTimer { nullptr };

    QList<DFMGLOBAL_NAMESPACE::ItemRoles> columnRoles;
    QMap<QString, bool> columnForRoleHiddenMap;

    QTimer *scrollBarValueChangedTimer { nullptr };
    bool scrollBarSliderPressed { false };

    bool pressedStartWithExpand { false };
    bool mouseLeftPressed { false };
    QPoint mouseLastPos { QPoint(0, 0) };
    QRect mouseMoveRect { QRect(-1, -1, 1, 1) };
    bool initHorizontalOffset { false };
    int columnCountByCalc { 0 };

    bool itemsExpandable { false };
    std::atomic_bool isShowSmbMountError { false };
    QString previousGroupStrategy { GroupStrategy::kNoGroup };

    explicit FileViewPrivate(FileView *qq);
    int iconModeColumnCount(int itemWidth = 0) const;
    int calcColumnCount(int widgetWidth, int itemWidth = 0) const;
    QUrl modelIndexUrl(const QModelIndex &index) const;

    void initIconModeView();
    void initListModeView();

    QModelIndexList selectedDraggableIndexes();

    void initContentLabel();
    void updateHorizontalScrollBarPosition();
    void pureResizeEvent(QResizeEvent *event);

    void loadViewMode(const QUrl &url);
    QVariant fileViewStateValue(const QUrl &url, const QString &key, const QVariant &defalutValue);

    void updateHorizontalOffset();
    // 使用 strategyName 是因为当 setGroup 时，model并不是分组状态
    void adjustHeaderLayoutMargin(const QString &strategyName);
    void adjustIconModeSpacing(const QString &strategyName);
};

}

#endif   // FILEVIEW_P_H
