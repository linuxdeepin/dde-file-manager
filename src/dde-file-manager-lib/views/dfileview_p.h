/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef DFILEVIEWPRIVATE_H
#define DFILEVIEWPRIVATE_H

#include <QResizeEvent>
#include <QActionGroup>
#include <QScrollBar>
#include <QPointer>
#include <QTimer>
#include <QMutex>
#include <QLabel>

#include <durl.h>
#include <danchors.h>

#include "dfileview.h"
#include "interfaces/dabstractfileinfo.h"
#include "dfilesystemmodel.h"
#include "dfmheaderview.h"

class DFMOpticalMediaWidget;
class DDiskManager;

class DFileViewPrivate
{
public:
    explicit DFileViewPrivate(DFileView *qq)
        : q_ptr(qq) {}

    int iconModeColumnCount(int itemWidth = 0) const;
    QVariant fileViewStateValue(const DUrl &url, const QString &key, const QVariant &defalutValue);
    void setFileViewStateValue(const DUrl &url, const QString &key, const QVariant &value);
    void updateHorizontalScrollBarPosition();
    void pureResizeEvent(QResizeEvent *event);
    void doFileNameColResize();
    void toggleHeaderViewSnap(bool on);
    void _q_onSectionHandleDoubleClicked(int logicalIndex);
    QModelIndexList selectedDraggableIndexes() const;
    QPixmap renderToPixmap(const QModelIndexList &indexes) const;
    void paintDragIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment = Qt::AlignCenter,
                       QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off) const;

public:
    DFileView *q_ptr;

    DFileMenuManager *fileMenuManager = nullptr;
    DFMHeaderView *headerView = nullptr;
    QWidget *headerViewHolder = nullptr;
    DFMOpticalMediaWidget *headerOpticalDisc = nullptr;
    DStatusBar *statusBar = nullptr;

    QActionGroup *displayAsActionGroup = nullptr;
    QActionGroup *sortByActionGroup = nullptr;
    QActionGroup *openWithActionGroup = nullptr;

    FileViewHelper *fileViewHelper = nullptr;

    QTimer *updateStatusBarTimer = nullptr;

    QScrollBar *verticalScrollBar = nullptr;

    DDiskManager *diskmgr = nullptr;

    QActionGroup *toolbarActionGroup = nullptr;


    // 用于实现触屏滚动视图和框选文件不冲突，手指在屏幕上按下短时间内就开始移动
    // 会被认为触发滚动视图，否则为触发文件选择（时间默认为300毫秒）
    QPointer<QTimer> updateEnableSelectionByMouseTimer;

    // 记录触摸按下事件，在mouse move事件中使用，用于判断手指移动的距离，当大于
    // QPlatformTheme::TouchDoubleTapDistance 的值时认为触发触屏滚动
    QPoint lastTouchBeginPos;

    QList<int> columnRoles;

    DFileView::ViewMode defaultViewMode = DFileView::IconMode;
    DFileView::ViewMode currentViewMode = DFileView::IconMode;
    /// move cursor later selecte index when pressed key shift
    QModelIndex lastCursorIndex;

    QModelIndex mouseLastPressedIndex;

    //记录当前mousePressEvent中按下的Index
    QModelIndex m_currentPressedIndex;

    QItemSelection currentSelection;

    /// drag drop
    QModelIndex dragMoveHoverIndex;
    //析构锁，当更新updatestatusbar正在处理时，不要析构
    QMutex m_mutexUpdateStatusBar;

    /// list mode column visible
    QMap<QString, bool> columnForRoleHiddenMap;

    DUrlList preSelectionUrls;

    /// Saved before sorting
    DUrlList oldSelectedUrls;

    DAnchors<QLabel> contentLabel = nullptr;

    DUrl oldCurrentUrl;

    /// menu actions filter
    QSet<MenuAction> menuWhitelist;

    QSet<MenuAction> menuBlacklist;

    QSet<DFileView::SelectionMode> enabledSelectionModes;

    int horizontalOffset = 0;
    int firstVisibleColumn = -1;
    int lastVisibleColumn = -1;
    int cachedViewWidth = -1;
    int touchTapDistance = -1;
    int showCount = 0;  //记录showEvent次数，为了在第一次时去调整列表模式的表头宽度
    DFileView::RandeIndex visibleIndexRande;

    bool allowedAdjustColumnSize = true;
    bool adjustFileNameCol = false; // mac finder style half-auto col size adjustment flag.

    char justAvoidWaringOfAlignmentBoundary[2] = {0};//只是为了避免边界对其问题警告，其他地方未使用。//若有更好的办法可以替换之

    bool isVaultDelSigConnected = false; //is vault delete signal connected.

    bool isAlwaysOpenInCurrentWindow = false;
    QAtomicInteger<bool> m_isDragging = false;
    QAtomicInteger<bool> m_isMouseMoveing = false;
    //正在设置当前的rooturl
    QAtomicInteger<bool> m_isSetRootUrl = false;
    QAtomicInteger<bool> m_isMouseLeftPress = false;
    Q_DECLARE_PUBLIC(DFileView)
};

#endif // DFILEVIEWPRIVATE_H
