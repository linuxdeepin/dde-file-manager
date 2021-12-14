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
}

const QUrl FileManagerWindow::rootUrl()
{
    return QUrl();
}

void FileManagerWindow::moveCenter(const QPoint &cp)
{
    QRect qr = frameGeometry();

    qr.moveCenter(cp);
    move(qr.topLeft());
}

void FileManagerWindow::setTitleBar(QWidget *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null TitleBar");
    titlebar()->setContentsMargins(0, 0, 0, 0);
    titlebar()->setCustomWidget(w);
}

void FileManagerWindow::setTitleMenu(QMenu *menu)
{
    Q_ASSERT_X(menu, "FileManagerWindow", "Null Title Menu");
    titlebar()->setMenu(menu);
}

void FileManagerWindow::setSideBar(QWidget *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null setSideBar");
}

void FileManagerWindow::initializeUi()
{
    titlebar()->setIcon(QIcon::fromTheme("dde-file-manager", QIcon::fromTheme("system-file-manager")));

    // size
    resize(d->kDefaultWindowWidth, d->kDefaultWindowHeight);
    setMinimumSize(d->kMinimumWindowWidth, d->kMinimumWindowHeight);
}

DFMBASE_END_NAMESPACE
