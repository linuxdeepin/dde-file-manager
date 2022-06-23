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

#include <DFontSizeManager>
#include <DStyle>

#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>

static constexpr int kNameMaxLength = 255;
static constexpr int kMenuBtnWidth = 18;
static constexpr int kMenuBtnHeight = 18;

DDP_ORGANIZER_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

CollectionTitleBarPrivate::CollectionTitleBarPrivate(CollectionTitleBar *qq)
    : q(qq)
{
    nameLabel = new DLabel(q);
    nameLabel->setWordWrap(false);

    nameLineEdit = new DLineEdit(q);
    nameLineEdit->lineEdit()->setMaxLength(kNameMaxLength);
    nameLineEdit->setClearButtonEnabled(false);
    DStyle::setFocusRectVisible(nameLineEdit->lineEdit(), false);
    QMargins margins = nameLineEdit->lineEdit()->textMargins();
    margins.setLeft(0);
    nameLineEdit->lineEdit()->setTextMargins(margins);

    QFont nameFont = nameLineEdit->font();
    nameFont = DFontSizeManager::instance()->get(DFontSizeManager::T8, nameFont);
    nameFont.setWeight(QFont::Medium);
    nameLabel->setFont(nameFont);
    nameLineEdit->setFont(nameFont);

    QPalette palette(nameLineEdit->palette());
    palette.setColor(QPalette::WindowText, Qt::black);
    nameLabel->setPalette(palette);

    palette.setColor(QPalette::Button, Qt::transparent);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Highlight, QColor(0x00, 0x61, 0xf7));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    nameLineEdit->setPalette(palette);

    nameWidget = new DStackedWidget(q);
    nameWidget->layout()->setContentsMargins(0, 0, 0, 0);

    nameWidget->addWidget(nameLabel);
    nameWidget->addWidget(nameLineEdit);
    nameWidget->setCurrentWidget(nameLabel);

    menuBtn = new DPushButton(q);
//    menuBtn->setIcon()
    menuBtn->setFixedSize(kMenuBtnWidth, kMenuBtnHeight);
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
    nameLineEdit->setFocus();
    nameLineEdit->lineEdit()->setSelection(0, nameLineEdit->lineEdit()->maxLength());
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

QString CollectionTitleBar::titleName() const
{
    return d->titleName;
}

bool CollectionTitleBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->nameWidget && QEvent::MouseButtonDblClick == event->type()) {
        d->modifyTitleName();
        return true;
    }

    return DBlurEffectWidget::eventFilter(obj, event);
}
