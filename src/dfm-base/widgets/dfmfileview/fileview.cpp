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

DFMBASE_BEGIN_NAMESPACE
FileView::FileView(QWidget *parent)
    : DListView(parent)
    , d_ptr(new FileViewPrivate(this))
{
    Q_D(FileView);
    auto model = new FileViewModel;
    setModel(model);
    this->setCornerWidget(d->headview);
}

void FileView::setViewMode(QListView::ViewMode mode)
{
    DListView::setViewMode(mode);
}

void FileView::setRootUrl(const QUrl &url)
{
    qobject_cast<FileViewModel *>(model())->setRootUrl(url);
}

QUrl FileView::rootUrl()
{
    return qobject_cast<FileViewModel *>(model())->rootUrl();
}

void FileView::resizeEvent(QResizeEvent *event)
{
    return DListView::resizeEvent(event);
}

DFMBASE_END_NAMESPACE

