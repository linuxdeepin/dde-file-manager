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

FileManagerWindowPrivate::FileManagerWindowPrivate(FileManagerWindow *qq)
    : QObject(nullptr), q(qq)
{
}

FileManagerWindow::FileManagerWindow(QWidget *parent)
    : DMainWindow(parent),
      d(new FileManagerWindowPrivate(this))
{
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

DFMBASE_END_NAMESPACE
