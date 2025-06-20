// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/widgets/filemanagerwindow.h>

namespace dfmbase {

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

QWidget *AbstractBaseView::contentWidget() const
{
    return nullptr;
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

}
