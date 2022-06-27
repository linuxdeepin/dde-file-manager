/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef FILEMANAGERWINDOW_H
#define FILEMANAGERWINDOW_H

#include "dfm-base/dfm_base_global.h"

#include <DMainWindow>

DWIDGET_USE_NAMESPACE

namespace dfmbase {

class AbstractFrame;
class FileManagerWindowPrivate;
class FileManagerWindow : public DMainWindow
{
    Q_OBJECT
    friend class FileManagerWindowPrivate;

public:
    explicit FileManagerWindow(const QUrl &url, QWidget *parent = nullptr);
    virtual ~FileManagerWindow() override;

    virtual void cd(const QUrl &url);
    virtual bool saveClosedSate() const;

    QUrl currentUrl() const;
    void moveCenter(const QPoint &cp);
    void installTitleBar(AbstractFrame *w);
    void installTitleMenu(QMenu *menu);
    void installSideBar(AbstractFrame *w);
    void installWorkSpace(AbstractFrame *w);
    void installDetailView(AbstractFrame *w);

    AbstractFrame *titleBar() const;
    AbstractFrame *sideBar() const;
    AbstractFrame *workSpace() const;
    AbstractFrame *detailView() const;

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void aboutToClose();
    void positionChanged(const QPoint &pos);
    void currentUrlChanged(const QUrl &url);
    void currentViewStateChanged();
    void selectUrlChanged(const QList<QUrl> &urlList);

    // The following signals are used to send shortcut requests
    // TODO(liuyangming): impl follow
    void reqRefresh();   // F5
    void reqActivateNextTab();   // ctrl + Tab
    void reqActivatePreviousTab();   // ctrl + {shift + } BackTab
    void reqSearchCtrlF();   // ctrl + F
    void reqSearchCtrlL();   // ctrl + L
    void reqBack();   // ctrl / alt + Left
    void reqForward();   // ctrl / alt + Right
    void reqCloseCurrentTab();   // ctrl + W
    void reqTriggerActionByIndex(int index);   // ctrl + [1, 8]
    void reqActivateTabByIndex(int index);   // alt + [1, 8]
    void reqShowHotkeyHelp();   // ctrl + shift + ?

    void titleBarInstallFinished();
    void titleMenuInstallFinished();
    void sideBarInstallFinished();
    void workspaceInstallFinished();
    void detailViewInstallFinished();

private:
    void initializeUi();
    void initConnect();

private:
    QScopedPointer<FileManagerWindowPrivate> d;
};

}

#endif   // FILEMANAGERWINDOW_H
