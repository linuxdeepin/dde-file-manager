/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "collectionwidget_p.h"
#include "collectiontitlebar.h"
#include "collectionview.h"

#include <QUrl>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

static constexpr int kTitleBarHeight = 36;

DWIDGET_USE_NAMESPACE
DDP_ORGANIZER_USE_NAMESPACE

CollectionWidgetPrivate::CollectionWidgetPrivate(CollectionWidget *qq)
    : q(qq)
{

}

CollectionWidgetPrivate::~CollectionWidgetPrivate()
{

}

CollectionWidget::CollectionWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , d(new CollectionWidgetPrivate(this))
{
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(0, 34, 109, static_cast<int>(0.2*255)));
    setMaskAlpha(static_cast<int>(0.2*255));

    d->view = new CollectionView(this);
    d->view->viewport()->installEventFilter(this);
    d->view->setGeometry(geometry());
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->addWidget(d->view);

    d->titleBar = new CollectionTitleBar(this);
    d->titleBar->move(0, 0);
    d->titleBar->setFixedHeight(kTitleBarHeight);
    d->titleBar->hide();

    this->setLayout(d->mainLayout);
}

CollectionWidget::~CollectionWidget()
{

}

void CollectionWidget::setModel(QAbstractItemModel *model)
{
    d->view->setModel(model);
}

void CollectionWidget::setUrls(const QList<QUrl> &urls)
{
    d->view->setUrls(urls);
}

QList<QUrl> CollectionWidget::urls() const
{
    return d->view->urls();
}

void CollectionWidget::setDragEnabled(bool enable)
{
    d->view->setDragEnabled(enable);
}

bool CollectionWidget::dragEnabled() const
{
    return d->view->dragEnabled();
}

void CollectionWidget::setTitleName(const QString &name)
{
    d->titleBar->setTitleName(name);
}

QString CollectionWidget::titleName() const
{
    return d->titleBar->titleName();
}

void CollectionWidget::setRenamable(const bool renamable)
{
    d->titleBar->setRenamable(renamable);
}

bool CollectionWidget::renamable() const
{
    return d->titleBar->renamable();
}

void CollectionWidget::resizeEvent(QResizeEvent *event)
{
    DBlurEffectWidget::resizeEvent(event);

    d->view->setGeometry(geometry());
    d->titleBar->setFixedWidth(width());
}

bool CollectionWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->view->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            mouseMoveEvent(mouseEvent);
        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            mousePressEvent(mouseEvent);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            mouseReleaseEvent(mouseEvent);
        }
    }

    return DBlurEffectWidget::eventFilter(obj, event);
}

void CollectionWidget::enterEvent(QEvent *event)
{
    d->titleBar->show();

    DBlurEffectWidget::enterEvent(event);
}

void CollectionWidget::leaveEvent(QEvent *event)
{
    d->titleBar->hide();

    DBlurEffectWidget::leaveEvent(event);
}
