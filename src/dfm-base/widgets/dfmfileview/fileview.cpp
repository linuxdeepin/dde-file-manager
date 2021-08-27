/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "headerview.h"
#include "fileview.h"
#include "private/fileview_p.h"

#include <QResizeEvent>
#include <QScrollBar>

DFMFileView::DFMFileView(QWidget *parent)
    : DListView(parent)
    , d_ptr(new DFMFileViewPrivate(this))
{
    Q_D(DFMFileView);
    auto model = new DFMFileViewModel;
    setModel(model);
    d_ptr->updateViewDelegate(viewMode());
    this->setCornerWidget(d->m_headview);
}

void DFMFileView::setViewMode(QListView::ViewMode mode)
{
    if (viewMode() == mode)
        return;
    d_ptr->updateViewDelegate(mode);
}

void DFMFileView::setRootUrl(const QUrl &url)
{
    qobject_cast<DFMFileViewModel *>(model())->setRootUrl(url);
}

QUrl DFMFileView::rootUrl()
{
    return qobject_cast<DFMFileViewModel *>(model())->rootUrl();
}

void DFMFileView::resizeEvent(QResizeEvent *event)
{
    return DListView::resizeEvent(event);
}

