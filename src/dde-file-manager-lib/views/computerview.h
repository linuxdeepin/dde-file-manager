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

#ifndef COMPUTERVIEW_H
#define COMPUTERVIEW_H

#include <QScrollArea>
#include <QResizeEvent>
#include <QTimerEvent>
#include <QStringList>
#include <QMouseEvent>
#include <QShowEvent>
#include <QIcon>
#include <QFrame>
#include <QListView>
#include <dflowlayout.h>
#include "dstyleditemdelegate.h"
#include "dfileview.h"

#include "fileitem.h"
#include "durl.h"
#include "dabstractfileinfo.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "dstatusbar.h"
#include "dfmbaseview.h"

DFM_USE_NAMESPACE

class ComputerListView;
class ComputerModel;
class DFMUrlBaseEvent;
class ComputerView : public QWidget, public DFMBaseView
{
    Q_OBJECT
public:
    explicit ComputerView(QWidget *parent = nullptr);
    ~ComputerView() override;

    QWidget* widget() const override;
    DUrl rootUrl() const override;
    bool setRootUrl(const DUrl &url) override;
    QListView *view();
    // 当前view是否在忙碌中
    bool isEventProcessing() const;
    // 设置当前需要析构
    void setNeedRelease();

public Q_SLOTS:
    void contextMenu(const QPoint &pos);
    void onRenameRequested(const DFMUrlBaseEvent &event);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    ComputerListView *m_view;
    ComputerModel *m_model;
    DStatusBar *m_statusbar;
    //是否是在事件处理中
    QAtomicInteger<bool> m_eventProcessing = false;
    QAtomicInteger<bool> m_needRelease = false;
    static const QList<int> iconsizes;
};

// sp3 feature 35 光盘加载时光标移动到光驱图标上，光标变成繁忙状态，其余时候为正常指针
// 原来的 view 直接使用的 QListView，因此这里继承重写其中的 mouseMoveEvent 函数
class ComputerListView : public QListView
{
    Q_OBJECT
public:
    explicit ComputerListView(QWidget *parent = nullptr);
    void closeEditingEditor();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

    void showEvent(QShowEvent *event) override;
};

#endif // COMPUTERVIEW_H
