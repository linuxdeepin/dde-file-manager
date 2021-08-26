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

#include "dfmheaderview.h"
#include "dfmfileview.h"
#include "private/dfmfileview_p.h"
#include "dfmfileviewmodel.h"

#include <QResizeEvent>
#include <QScrollBar>

#define ICON_VIEW_SPACING 5
#define LIST_VIEW_SPACING 1
#define LIST_VIEW_MINIMUM_WIDTH 80
#define LIST_VIEW_ICON_SIZE 24


DFMFileView::DFMFileView(QWidget *parent)
    : DListView(parent)
    , d_ptr(new DFMFileViewPrivate(this))
{
    Q_D(DFMFileView);
    auto model = new DFMFileViewModel;
    setModel(model);
    updateViewDelegate(viewMode());
    this->setCornerWidget(d->m_headview);
}

void DFMFileView::setViewMode(QListView::ViewMode mode)
{
    if (viewMode() == mode)
        return;
    updateViewDelegate(mode);
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
void DFMFileView::updateViewDelegate(const QListView::ViewMode &mode){
    Q_D(DFMFileView);
    if (QListView::ListMode == mode) {
        d->m_headview->show();
        if (!d->m_listDelegate)
            d->m_listDelegate = new DFMListItemDelegate(this);
        setItemDelegate(d->m_listDelegate);
        setIconSize(QSize(LIST_VIEW_ICON_SIZE, LIST_VIEW_ICON_SIZE));
    }
    else {
        d->m_headview->hide();
        if (!d->m_iconDelegate)
            d->m_iconDelegate = new DFMIconItemDelegate(this);
        setItemDelegate(d->m_iconDelegate);
        setIconSize(d->m_iconDelegate->iconSizeByIconSizeLevel());
    }
    QListView::setViewMode(mode);
}

DFMFileViewPrivate::DFMFileViewPrivate(DFMFileView *qq)
    : q_ptr(qq)
    , m_headview(new DFMHeaderView(Qt::Orientation::Horizontal,qq))
{

}

int DFMFileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    Q_Q(const DFMFileView);

    //    int frameAroundContents = 0;
    //    if (q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents))
//        frameAroundContents = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;

    int horizontalMargin = /*q->verticalScrollBarPolicy()==Qt::ScrollBarAsNeeded
            ? q->style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, q->verticalScrollBar()) + frameAroundContents
            : */0;

    int contentWidth = q->maximumViewportSize().width();

//    if (itemWidth <= 0)
//        itemWidth = q->itemSizeHint().width() + q->spacing() * 2;

    return qMax((contentWidth - horizontalMargin - 1) / itemWidth, 1);
}
