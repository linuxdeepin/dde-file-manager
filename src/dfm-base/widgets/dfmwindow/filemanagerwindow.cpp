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

void FileManagerWindow::setRootUrl(const QUrl &url)
{
    if (d->titleBar)
        d->titleBar->setCurrentUrl(url);
    if (d->sideBar)
        d->sideBar->setCurrentUrl(url);
    if (d->workspace)
        d->workspace->setCurrentUrl(url);
}

const QUrl FileManagerWindow::rootUrl() const
{
    return d->currentUrl;
}

void FileManagerWindow::moveCenter(const QPoint &cp)
{
    QRect qr = frameGeometry();

    qr.moveCenter(cp);
    move(qr.topLeft());
}

void FileManagerWindow::setTitleBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null TitleBar");
    d->titleBar = w;
    w->setCurrentUrl(d->currentUrl);
    titlebar()->setCustomWidget(w);
}

void FileManagerWindow::setTitleMenu(QMenu *menu)
{
    Q_ASSERT_X(menu, "FileManagerWindow", "Null Title Menu");
    titlebar()->setMenu(menu);
}

void FileManagerWindow::setSideBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null setSideBar");
    d->sideBar = w;
    w->setCurrentUrl(d->currentUrl);
    d->splitter->replaceWidget(0, w);

    w->setMaximumWidth(d->kMaximumLeftWidth);
    w->setMinimumWidth(d->kMinimumLeftWidth);
}

void FileManagerWindow::setWorkSpace(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null Workspace");
    d->workspace = w;
    w->setCurrentUrl(d->currentUrl);
    d->splitter->replaceWidget(1, w);

    QSizePolicy sp = w->sizePolicy();
    sp.setHorizontalStretch(1);
    w->setSizePolicy(sp);
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
    d->leftView = new QFrame(this);

    // right view
    d->rightView = new QFrame(this);

    // splitter
    d->splitter = new Splitter(Qt::Orientation::Horizontal, this);
    d->splitter->setChildrenCollapsible(false);
    d->splitter->setHandleWidth(0);
    d->splitter->addWidget(d->leftView);
    d->splitter->addWidget(d->rightView);
    setCentralWidget(d->splitter);
}

DFMBASE_END_NAMESPACE
