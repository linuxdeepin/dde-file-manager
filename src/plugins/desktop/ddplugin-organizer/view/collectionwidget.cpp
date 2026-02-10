// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionwidget_p.h"
#include "collectiontitlebar.h"
#include "collectionview.h"
#include "mode/collectiondataprovider.h"
#include "config/configpresenter.h"

#include <DGuiApplicationHelper>

#include <QUrl>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>

static constexpr int kTitleBarHeight = 24;
static constexpr int kWidgetRoundRadius = 8;

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

using namespace ddplugin_organizer;

CollectionWidgetPrivate::CollectionWidgetPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionWidget *qq, QObject *parent)
    : QObject(parent), q(qq), id(uuid), provider(dataProvider)
{
    connect(provider, &CollectionDataProvider::nameChanged, this, &CollectionWidgetPrivate::onNameChanged);
    connect(&updateSnapshotTimer, &QTimer::timeout, this, [this] {
        if (freeze) return;
        if (!q->isVisible()) return;
        // grab the DBlurWidget, the pixmap a little white, reduce the alpha channel to make the color colser.
        // if DTK fixed, then remove the color sets.
        auto colorNormal = q->maskColor();
        auto colorNew = colorNormal;
        colorNew.setAlpha(0.06 * 255);
        q->setMaskColor(colorNew);
        q->update();
        freezePixmap = q->grab();
        q->setMaskColor(colorNormal);
    });
    updateSnapshotTimer.setSingleShot(true);
    updateSnapshotTimer.setInterval(100);
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
    : DBlurEffectWidget(parent), d(new CollectionWidgetPrivate(uuid, dataProvider, this))
{
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    updateMaskColor();
    setBlurRectXRadius(kWidgetRoundRadius);
    setBlurRectYRadius(kWidgetRoundRadius);

    d->view = new CollectionView(uuid, dataProvider, this);
    d->view->viewport()->installEventFilter(this);
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(1, 1, 1, 1);   // for inner border
    d->mainLayout->addSpacing(16);
    d->mainLayout->addWidget(d->view);

    d->titleBar = new CollectionTitleBar(uuid, this);
    d->titleBar->move(0, 0);
    d->titleBar->setFixedHeight(kTitleBarHeight);
    d->titleBar->hide();

    this->setLayout(d->mainLayout);

    connect(d->titleBar, &CollectionTitleBar::sigRequestClose, this, &CollectionWidget::sigRequestClose, Qt::QueuedConnection);
    connect(d->titleBar, &CollectionTitleBar::sigRequestAdjustSizeMode, this, &CollectionWidget::sigRequestAdjustSizeMode, Qt::DirectConnection);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &CollectionWidget::updateMaskColor);
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

void CollectionWidget::setFreeze(bool freeze)
{
    d->freeze = freeze;
    d->view->setFreeze(freeze);
}

void CollectionWidget::cacheSnapshot()
{
    if (CfgPresenter->optimizeMovingPerformance())
        d->updateSnapshotTimer.start();
}

void CollectionWidget::updateMaskColor()
{
    QColor bgColor;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        bgColor = QColor(210, 210, 210, static_cast<int>(0.3 * 255));   // #D2D2D2 30%
    else
        bgColor = QColor(47, 47, 47, static_cast<int>(0.3 * 255));   // #2F2F2F 30%

    setMaskColor(bgColor);
    setMaskAlpha(bgColor.alpha());
}

void CollectionWidget::resizeEvent(QResizeEvent *event)
{
    DBlurEffectWidget::resizeEvent(event);

    d->titleBar->setFixedWidth(width());
}

bool CollectionWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->view->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            mouseMoveEvent(mouseEvent);
        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            mousePressEvent(mouseEvent);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            mouseReleaseEvent(mouseEvent);
        }
    }

    return DBlurEffectWidget::eventFilter(obj, event);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void CollectionWidget::enterEvent(QEvent *event)
#else
void CollectionWidget::enterEvent(QEnterEvent *event)
#endif
{
    d->titleBar->setTitleBarVisible(true);

    DBlurEffectWidget::enterEvent(event);
}

void CollectionWidget::leaveEvent(QEvent *event)
{
    d->titleBar->setTitleBarVisible(false);
    DBlurEffectWidget::leaveEvent(event);
}

void CollectionWidget::paintEvent(QPaintEvent *event)
{
    if (d->freeze && !d->freezePixmap.isNull()) {
        QPainter p(this);
        p.setPen(Qt::transparent);
        p.setBrush(Qt::transparent);
        p.drawPixmap(this->rect(), d->freezePixmap);
        return;
    }

    DBlurEffectWidget::paintEvent(event);

    // inner order
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, static_cast<int>(255 * 0.1)));   // #ffffff 10%

    const QRect rect(QPoint(0, 0), size());
    QPainterPath out;
    out.addRoundedRect(rect, kWidgetRoundRadius, kWidgetRoundRadius);
    QPainterPath in;
    in.addRoundedRect(rect.marginsRemoved(QMargins(1, 1, 1, 1)), kWidgetRoundRadius, kWidgetRoundRadius);
    p.drawPath(out - in);
}
