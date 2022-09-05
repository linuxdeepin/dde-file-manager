// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
