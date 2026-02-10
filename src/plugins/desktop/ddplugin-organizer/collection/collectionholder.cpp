// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionholder_p.h"
#include "view/collectionframe.h"
#include "view/collectionwidget.h"
#include "view/collectionview.h"
#include "config/configpresenter.h"
#include "models/collectionmodel.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE

CollectionHolderPrivate::CollectionHolderPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionHolder *qq, QObject *parent)
    : QObject(qq), q(qq), id(uuid), provider(dataProvider)
{
}

CollectionHolderPrivate::~CollectionHolderPrivate()
{
    if (frame) {
        delete frame;
        frame = nullptr;
    }
}

void CollectionHolderPrivate::onAdjustFrameSizeMode(const CollectionFrameSize &size)
{
    sizeMode = size;
    widget->setCollectionSize(size);
    emit q->styleChanged(id);
}

CollectionHolder::CollectionHolder(const QString &uuid, ddplugin_organizer::CollectionDataProvider *dataProvider, QObject *parent)
    : QObject(parent), d(new CollectionHolderPrivate(uuid, dataProvider, this))
{
    d->styleTimer.setSingleShot(true);
    d->styleTimer.setInterval(500);

    connect(&d->styleTimer, &QTimer::timeout, this, [this]() {
        emit styleChanged(id());
    });
}

CollectionHolder::~CollectionHolder()
{
}

QString CollectionHolder::id() const
{
    return d->id;
}

QString CollectionHolder::name()
{
    return d->widget->titleName();
}

void CollectionHolder::setName(const QString &text)
{
    d->widget->setTitleName(text);
}

DFrame *CollectionHolder::frame() const
{
    return d->frame;
}

CollectionWidget *CollectionHolder::widget() const
{
    return d->widget;
}

CollectionView *CollectionHolder::itemView() const
{
    return d->widget ? d->widget->view() : nullptr;
}

void CollectionHolder::createFrame(Surface *surface, CollectionModel *model)
{
    d->surface = surface;

    d->frame = new CollectionFrame(surface);

    d->model = model;
    d->widget = new CollectionWidget(d->id, d->provider, d->frame);
    d->widget->view()->setModel(d->model);
    d->widget->setGeometry(QRect(QPoint(0, 0), d->frame->size()));

    d->frame->setWidget(d->widget);

    connect(d->widget, &CollectionWidget::sigRequestClose, this, &CollectionHolder::sigRequestClose);
    connect(d->widget, &CollectionWidget::sigRequestAdjustSizeMode, d->frame, &CollectionFrame::adjustSizeMode);
    connect(d->frame, &CollectionFrame::sizeModeChanged, d.data(), &CollectionHolderPrivate::onAdjustFrameSizeMode);
    connect(d->frame, &CollectionFrame::surfaceChanged, this, &CollectionHolder::frameSurfaceChanged);
    connect(d->frame, &CollectionFrame::geometryChanged, this, [this]() {
        d->styleTimer.start();
    });
}

void CollectionHolder::setSurface(Surface *surface)
{
    d->surface = surface;

    if (d->frame)
        d->frame->setParent(surface);
}

Surface *CollectionHolder::surface() const
{
    return d->surface;
}

void CollectionHolder::show()
{
    d->frame->show();
    d->frame->raise();
}

void CollectionHolder::openEditor(const QUrl &url)
{
    d->widget->view()->openEditor(url);
}

void CollectionHolder::selectUrl(const QUrl &url, const QItemSelectionModel::SelectionFlag &flags)
{
    d->widget->view()->selectUrl(url, flags);
}

void CollectionHolder::setMovable(const bool movable)
{
    auto features = d->frame->collectionFeatures();
    if (movable)
        features |= CollectionFrame::CollectionFrameMovable;
    else
        features &= ~CollectionFrame::CollectionFrameMovable;

    d->frame->setCollectionFeatures(features);
}

bool CollectionHolder::movable() const
{
    return d->frame->collectionFeatures().testFlag(CollectionFrame::CollectionFrameMovable);
}

void CollectionHolder::setClosable(const bool closable)
{
    auto features = d->frame->collectionFeatures();
    if (closable)
        features |= CollectionFrame::CollectionFrameClosable;
    else
        features &= ~CollectionFrame::CollectionFrameClosable;

    d->frame->setCollectionFeatures(features);
    d->widget->setClosable(closable);
}

bool CollectionHolder::closable() const
{
    return d->frame->collectionFeatures().testFlag(CollectionFrame::CollectionFrameClosable);
}

void CollectionHolder::setFloatable(const bool floatable)
{
    auto features = d->frame->collectionFeatures();
    if (floatable)
        features |= CollectionFrame::CollectionFrameFloatable;
    else
        features &= ~CollectionFrame::CollectionFrameFloatable;

    d->frame->setCollectionFeatures(features);
}

bool CollectionHolder::floatable() const
{
    return d->frame->collectionFeatures().testFlag(CollectionFrame::CollectionFrameFloatable);
}

void CollectionHolder::setHiddableCollection(const bool hiddable)
{
    auto features = d->frame->collectionFeatures();
    if (hiddable)
        features |= CollectionFrame::CollectionFrameHiddable;
    else
        features &= ~CollectionFrame::CollectionFrameHiddable;

    d->frame->setCollectionFeatures(features);
}

bool CollectionHolder::hiddableCollection() const
{
    return d->frame->collectionFeatures().testFlag(CollectionFrame::CollectionFrameHiddable);
}

void CollectionHolder::setStretchable(const bool stretchable)
{
    auto features = d->frame->collectionFeatures();
    if (stretchable)
        features |= CollectionFrame::CollectionFrameStretchable;
    else
        features &= ~CollectionFrame::CollectionFrameStretchable;

    d->frame->setCollectionFeatures(features);
}

bool CollectionHolder::stretchable() const
{
    return d->frame->collectionFeatures().testFlag(CollectionFrame::CollectionFrameStretchable);
}

void CollectionHolder::setAdjustable(const bool adjustable)
{
    auto features = d->frame->collectionFeatures();
    if (adjustable)
        features |= CollectionFrame::CollectionFrameAdjustable;
    else
        features &= ~CollectionFrame::CollectionFrameAdjustable;

    d->frame->setCollectionFeatures(features);
    d->widget->setAdjustable(adjustable);
}

bool CollectionHolder::adjustable() const
{
    return d->frame->collectionFeatures().testFlag(CollectionFrame::CollectionFrameAdjustable);
}

void CollectionHolder::setHiddableTitleBar(const bool hiddable)
{
    // todo
}

bool CollectionHolder::hiddableTitleBar() const
{
    // todo
    return false;
}

void CollectionHolder::setHiddableView(const bool hiddable)
{
    // todo
}

bool CollectionHolder::hiddableView() const
{
    // todo
    return false;
}

void CollectionHolder::setRenamable(const bool renamable)
{
    d->widget->setRenamable(renamable);
}

bool CollectionHolder::renamable() const
{
    return d->widget->renamable();
}

void CollectionHolder::setFileShiftable(bool enable)
{
    d->widget->view()->setFileShiftable(enable);
}

bool CollectionHolder::fileShiftable() const
{
    return d->widget->view()->fileShiftable();
}

QPropertyAnimation *CollectionHolder::createAnimation()
{
    QPropertyAnimation *ani = new QPropertyAnimation(d->frame, "pos");
    auto pos = d->frame->pos();
    ani->setDuration(500);
    ani->setEasingCurve(QEasingCurve::BezierSpline);
    ani->setStartValue(pos);
    ani->setEndValue(pos);
    ani->setKeyValueAt(0.2, pos + QPoint { -10, 0 });
    ani->setKeyValueAt(0.4, pos + QPoint { 10, 0 });
    ani->setKeyValueAt(0.6, pos + QPoint { -10, 0 });
    ani->setKeyValueAt(0.8, pos);
    return ani;
}

void CollectionHolder::selectFiles(const QList<QUrl> &urls)
{
    if (!itemView()) return;
    itemView()->selectUrls(urls);
    itemView()->scrollToBottom();
}

void CollectionHolder::setFreeze(bool freeze)
{
    if (CfgPresenter->optimizeMovingPerformance())
        d->widget->setFreeze(freeze);
}

void CollectionHolder::setStyle(const CollectionStyle &style)
{
    if (style.key != id())
        return;

    if (style.rect.isValid())
        d->frame->setGeometry(style.rect);
    d->widget->setCollectionSize(style.sizeMode);
    d->screenIndex = style.screenIndex;
    d->sizeMode = style.sizeMode;
}

CollectionStyle CollectionHolder::style() const
{
    CollectionStyle style;
    style.key = id();
    // todo get index.
    style.screenIndex = d->screenIndex;

    style.rect = d->frame->geometry();
    style.sizeMode = d->sizeMode;
    return style;
}
