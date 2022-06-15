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
#include "collectiontitlebar_p.h"

#include <QEvent>
#include <QFont>
#include <QFontMetrics>

#include <QLabel>
#include <QImage>
#include <QBackingStore>
#include <QRect>
#include <QPaintEvent>
#include <qpa/qplatformbackingstore.h>
#include "private/qpixmapfilter_p.h"
#include <private/qwidgetbackingstore_p.h>
#include <private/qwidget_p.h>
#include <QSharedPointer>
#include <QGraphicsBlurEffect>

DDP_ORGANIZER_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

CollectionTitleBarPrivate::CollectionTitleBarPrivate(CollectionTitleBar *qq)
    : q(qq)
{
    nameLabel = new DLabel(q);
    QFont font(nameLabel->font());
    font.setWeight(QFont::Medium);
    nameLabel->setFont(font);

    nameLineEdit = new DLineEdit(q);
    nameWidget = new DStackedWidget(q);
    nameWidget->addWidget(nameLabel);
    nameWidget->addWidget(nameLineEdit);
    nameWidget->setCurrentWidget(nameLabel);

    menuBtn = new DPushButton(q);
//    menuBtn->setIcon()
    menuBtn->setFixedSize(18, 18);
    menuBtn->setText("...");

    mainLayout = new QHBoxLayout(q);
    mainLayout->setContentsMargins(10, 0, 12, 0);
    mainLayout->setSpacing(12);
    mainLayout->addWidget(nameWidget);
    mainLayout->addWidget(menuBtn);
    q->setLayout(mainLayout);

    connect(nameLineEdit, &DLineEdit::editingFinished, this, &CollectionTitleBarPrivate::titleNameModified);
}

CollectionTitleBarPrivate::~CollectionTitleBarPrivate()
{

}

void CollectionTitleBarPrivate::modifyTitleName()
{
    if (!canRename)
        return;

    nameWidget->setCurrentWidget(nameLineEdit);
    nameLineEdit->setText(titleName);
}

void CollectionTitleBarPrivate::titleNameModified()
{
    if (nameLineEdit->text().trimmed().isEmpty())
        return;
    titleName = nameLineEdit->text().trimmed();

    updateDisplayName();
}

void CollectionTitleBarPrivate::updateDisplayName()
{
    nameWidget->setCurrentWidget(nameLabel);

    QFontMetrics fontMetrices(nameLabel->font());
    QString showName = fontMetrices.elidedText(titleName, Qt::ElideRight, nameLabel->width());
    nameLabel->setText(showName);
    nameLabel->setToolTip(titleName);
}

CollectionTitleBar::CollectionTitleBar(QWidget *parent)
    : DBlurEffectWidget(parent)
    , d(new CollectionTitleBarPrivate(this))
{
    setObjectName("titleBar");
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(0, 31, 119, static_cast<int>(0.2*255)));

    d->nameWidget->installEventFilter(this);
}

CollectionTitleBar::~CollectionTitleBar()
{

}

void CollectionTitleBar::setRenamable(const bool renamable)
{
    d->canRename = renamable;
}

bool CollectionTitleBar::renamable() const
{
    return d->canRename;
}

void CollectionTitleBar::setTitleName(const QString &name)
{
    if (d->titleName == name)
        return;

    d->titleName = name;
    d->updateDisplayName();
}

bool CollectionTitleBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->nameWidget && QEvent::MouseButtonDblClick == event->type()) {
        d->modifyTitleName();
        return true;
    }

    return DBlurEffectWidget::eventFilter(obj, event);
}
