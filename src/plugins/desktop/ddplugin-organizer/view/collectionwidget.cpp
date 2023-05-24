// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionwidget_p.h"
#include "collectiontitlebar.h"
#include "collectionview.h"
#include "mode/collectiondataprovider.h"

#include <QUrl>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

static constexpr int kTitleBarHeight = 24;
static constexpr int kWidgetRoundRadius = 8;

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

CollectionWidgetPrivate::CollectionWidgetPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionWidget *qq, QObject *parent)
    : QObject(parent)
    , q(qq)
    , id(uuid)
    , provider(dataProvider)
{
    connect(provider, &CollectionDataProvider::nameChanged, this, &CollectionWidgetPrivate::onNameChanged);
}

CollectionWidgetPrivate::~CollectionWidgetPrivate()
{

}

void CollectionWidgetPrivate::onNameChanged(const QString &key, const QString &name)
{
    if (key != id)
        return;
    titleBar->setTitleName(name);
}

CollectionWidget::CollectionWidget(const QString &uuid, ddplugin_organizer::CollectionDataProvider *dataProvider, QWidget *parent)
    : DBlurEffectWidget(parent)
    , d(new CollectionWidgetPrivate(uuid, dataProvider, this))
{
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(0, 47, 255, static_cast<int>(0.1 * 255)));
    setMaskAlpha(static_cast<int>(0.1 * 255));
    setBlurRectXRadius(kWidgetRoundRadius);
    setBlurRectYRadius(kWidgetRoundRadius);

    d->view = new CollectionView(uuid, dataProvider, this);
    d->view->viewport()->installEventFilter(this);
    d->view->setGeometry(geometry());
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->addWidget(d->view);

    d->titleBar = new CollectionTitleBar(uuid, this);
    d->titleBar->move(0, 0);
    d->titleBar->setFixedHeight(kTitleBarHeight);
    d->titleBar->hide();

    this->setLayout(d->mainLayout);

    connect(d->titleBar, &CollectionTitleBar::sigRequestClose, this, &CollectionWidget::sigRequestClose, Qt::QueuedConnection);
    connect(d->titleBar, &CollectionTitleBar::sigRequestAdjustSizeMode, this, &CollectionWidget::sigRequestAdjustSizeMode, Qt::DirectConnection);
}

CollectionWidget::~CollectionWidget()
{

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

void CollectionWidget::setClosable(const bool closable)
{
    d->titleBar->setClosable(closable);
}

bool CollectionWidget::closable() const
{
    return d->titleBar->closable();
}

void CollectionWidget::setAdjustable(const bool adjustable)
{
    d->titleBar->setAdjustable(adjustable);
}

bool CollectionWidget::adjustable() const
{
    return d->titleBar->adjustable();
}

void CollectionWidget::setCollectionSize(const CollectionFrameSize &size)
{
    d->titleBar->setCollectionSize(size);
}

CollectionFrameSize CollectionWidget::collectionSize() const
{
    return d->titleBar->collectionSize();
}

CollectionView *CollectionWidget::view() const
{
    return d->view;
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
    d->titleBar->setTitleBarVisible(true);

    DBlurEffectWidget::enterEvent(event);
}

void CollectionWidget::leaveEvent(QEvent *event)
{

    d->titleBar->setTitleBarVisible(false);

    DBlurEffectWidget::leaveEvent(event);
}
