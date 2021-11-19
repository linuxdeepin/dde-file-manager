/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DFILEMANAGERWINDOWPRIVATE_H
#define DFILEMANAGERWINDOWPRIVATE_H

#include "dfmglobal.h"
#include "dfmbaseview.h"
#include "dtabbar.h"
#include "dfmsidebar.h"
#include "dfmrightdetailview.h"
#include "dtoolbar.h"
#include "diconbutton.h"
#include "dfmsplitter.h"
#include "DHorizontalLine"
#include "drenamebar.h"
#include "dfmadvancesearchbar.h"

#include <QVBoxLayout>
#include <QStackedLayout>

DFM_USE_NAMESPACE

class DFileManagerWindow;
class DFileManagerWindowPrivate
{
public:
    enum vaultRemove{
        SUCCESS, //! 移动成功
        FAILED, //! 移动失败
        NOTEXIST     //! 文件不存在
    };

    explicit DFileManagerWindowPrivate(DFileManagerWindow *qq)
        : q_ptr(qq) {}

    void setCurrentView(DFMBaseView *view);
    bool processKeyPressEvent(QKeyEvent *event);
    bool processTitleBarEvent(QMouseEvent *event);
    bool cdForTab(Tab *tab, const DUrl &fileUrl);
    void initAdvanceSearchBar();
    bool isAdvanceSearchBarVisible() const;
    void setAdvanceSearchBarVisible(bool visible);
    void initRenameBar();
    bool isRenameBarVisible() const;
    void setRenameBarVisible(bool visible);
    void resetRenameBar();
    void storeUrlListToRenameBar(const QList<DUrl> &list) noexcept;
    void updateSelectUrl();
    void createRightDetailViewHolder();
    /**
     * @brief moveVaultPath 移动保险箱存储路径
     * @return 返回REMOVEOK移动成功，REMOVENO失败，EXIST文件不存在
     */
    DFileManagerWindowPrivate::vaultRemove moveVaultPath();

    QFrame *centralWidget{ nullptr };//中央区域（所有的除顶部区域）
    DFMSideBar *sideBar{ nullptr };
    QFrame *rightView { nullptr };
    DFMRightDetailView *detailView { nullptr };
    QFrame *rightDetailViewHolder { nullptr };
    QHBoxLayout *midLayout { nullptr };
    QVBoxLayout *rightViewLayout { nullptr };
    DToolBar *toolbar{ nullptr };
    TabBar *tabBar { nullptr };
    DIconButton *newTabButton { nullptr };
    QFrame *tabTopLine { nullptr };
    QFrame *tabBottomLine { nullptr };
    DFMBaseView *currentView { nullptr };
    DStatusBar *statusBar { nullptr };
    QVBoxLayout *mainLayout { nullptr };
    DFMSplitter *splitter { nullptr };
    QFrame *titleFrame { nullptr };
    QStackedLayout *viewStackLayout { nullptr };//右边文件显示的布局
    QFrame *emptyTrashHolder { nullptr };
    DHorizontalLine *emptyTrashSplitLine { nullptr };
    DRenameBar *renameBar{ nullptr };
    DFMAdvanceSearchBar *advanceSearchBar = nullptr;
    QScrollArea *advanceSearchArea = nullptr;

    QMap<DUrl, QWidget *> views;

    bool move = false;
    QPoint startPoint;
    QPoint windowPoint;
    //是否需要关闭
    QAtomicInteger<bool> m_isNeedClosed = false;

    DFileManagerWindow *q_ptr{ nullptr };

    D_DECLARE_PUBLIC(DFileManagerWindow)
};

#endif // DFILEMANAGERWINDOWPRIVATE_H
