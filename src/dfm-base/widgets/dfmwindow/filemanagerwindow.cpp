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
#include "filemanagerwindow.h"

#include "private/filemanagerwindow_p.h"

#include <QUrl>

DFMBASE_BEGIN_NAMESPACE

/*!
 * \class FileManagerWindowPrivate
 * \brief
 */

FileManagerWindowPrivate::FileManagerWindowPrivate(const QUrl &url, FileManagerWindow *qq)
    : QObject(nullptr),
      q(qq),
      currentUrl(url)
{
}

/*!
 * \class FileManagerWindow
 * \brief
 */

FileManagerWindow::FileManagerWindow(const QUrl &url, QWidget *parent)
    : DMainWindow(parent),
      d(new FileManagerWindowPrivate(url, this))
{
    initializeUi();
}

FileManagerWindow::~FileManagerWindow()
{
}

void FileManagerWindow::cd(const QUrl &url)
{
    d->currentUrl = url;
    if (d->titleBar)
        d->titleBar->setCurrentUrl(url);
    if (d->sideBar)
        d->sideBar->setCurrentUrl(url);
    if (d->workspace)
        d->workspace->setCurrentUrl(url);
}

QUrl FileManagerWindow::currentUrl() const
{
    return d->currentUrl;
}

void FileManagerWindow::moveCenter(const QPoint &cp)
{
    QRect qr = frameGeometry();

    qr.moveCenter(cp);
    move(qr.topLeft());
}

void FileManagerWindow::installTitleBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null TitleBar");
    d->titleBar = w;
    d->titleBar->setCurrentUrl(d->currentUrl);
    titlebar()->setCustomWidget(d->titleBar);
}

void FileManagerWindow::installTitleMenu(QMenu *menu)
{
    Q_ASSERT_X(menu, "FileManagerWindow", "Null Title Menu");
    titlebar()->setMenu(menu);
}

void FileManagerWindow::installSideBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null setSideBar");
    d->sideBar = w;
    d->sideBar->setCurrentUrl(d->currentUrl);
    d->splitter->replaceWidget(0, d->sideBar);

    d->sideBar->setContentsMargins(0, 0, 0, 0);
    d->sideBar->setMaximumWidth(d->kMaximumLeftWidth);
    d->sideBar->setMinimumWidth(d->kMinimumLeftWidth);

    // connections
    connect(this, &FileManagerWindow::currentUrlChanged, this, [this]() {
        d->sideBar->setCurrentUrl(d->currentUrl);
    });
}

void FileManagerWindow::installWorkSpace(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null Workspace");
    d->workspace = w;
    d->workspace->setCurrentUrl(d->currentUrl);
    d->splitter->replaceWidget(1, d->workspace);

    //NOTE(zccrs): 保证窗口宽度改变时只会调整right view的宽度，侧边栏保持不变
    //             QSplitter是使用QLayout的策略对widgets进行布局，所以此处
    //             设置size policy可以生效
    QSizePolicy sp = d->workspace->sizePolicy();
    sp.setHorizontalStretch(1);
    d->workspace->setSizePolicy(sp);
}

AbstractFrame *FileManagerWindow::titleBar() const
{
    return d->titleBar;
}

AbstractFrame *FileManagerWindow::sideBar() const
{
    return d->sideBar;
}

AbstractFrame *FileManagerWindow::workSpace() const
{
    return d->workspace;
}

void FileManagerWindow::initializeUi()
{
    titlebar()->setIcon(QIcon::fromTheme("dde-file-manager", QIcon::fromTheme("system-file-manager")));

    // size
    resize(d->kDefaultWindowWidth, d->kDefaultWindowHeight);
    setMinimumSize(d->kMinimumWindowWidth, d->kMinimumWindowHeight);

    // title bar
    titlebar()->setContentsMargins(0, 0, 0, 0);

    // left view
    d->leftView = new QFrame;
    d->leftView->setMaximumWidth(d->kMaximumLeftWidth);
    d->leftView->setMinimumWidth(d->kMinimumLeftWidth);

    // right view
    d->rightView = new QFrame;

    // splitter
    d->splitter = new Splitter(Qt::Orientation::Horizontal, this);
    d->splitter->setChildrenCollapsible(false);
    d->splitter->setHandleWidth(0);
    d->splitter->addWidget(d->leftView);
    d->splitter->addWidget(d->rightView);
    setCentralWidget(d->splitter);
}

DFMBASE_END_NAMESPACE
