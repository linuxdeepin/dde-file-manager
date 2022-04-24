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
#include "abstractbaseview.h"

#include "widgets/dfmwindow/filemanagerwindow.h"

DFMBASE_BEGIN_NAMESPACE

/*!
 * \class BaseView
 *
 * \brief
 */

AbstractBaseView::AbstractBaseView()
{
}

AbstractBaseView::~AbstractBaseView()
{
}

void AbstractBaseView::deleteLater()
{
}

AbstractBaseView::ViewState AbstractBaseView::viewState() const
{
    return ViewState::kViewIdle;
}

QList<QAction *> AbstractBaseView::toolBarActionList() const
{
    return QList<QAction *>();
}

QList<QUrl> AbstractBaseView::selectedUrlList() const
{
    return QList<QUrl>();
}

void AbstractBaseView::refresh()
{
}

void AbstractBaseView::notifyChangeUrl()
{
    if (FileManagerWindow *w = qobject_cast<FileManagerWindow *>(widget()->window()))
        emit w->changeCurrentUrl();
}

void AbstractBaseView::notifyStateChanged()
{
    if (FileManagerWindow *w = qobject_cast<FileManagerWindow *>(widget()->window()))
        emit w->currentViewStateChanged();
}

void AbstractBaseView::requestCdTo(const QUrl &url)
{
    if (FileManagerWindow *w = qobject_cast<FileManagerWindow *>(widget()->window()))
        w->cd(url);
}

void AbstractBaseView::notifySelectUrlChanged(const QList<QUrl> &urlList)
{
    if (FileManagerWindow *w = qobject_cast<FileManagerWindow *>(widget()->window()))
        emit w->selectUrlChanged(urlList);
}

DFMBASE_END_NAMESPACE
