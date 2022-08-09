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

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE

CollectionTitleBarPrivate::CollectionTitleBarPrivate(const QString &uuid, CollectionTitleBar *qq)
    : q(qq)
    , id(uuid)
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
    connect(menuBtn, &DPushButton::clicked, this, &CollectionTitleBarPrivate::showMenu);

    menu = new DMenu(q);
    connect(menu, &DMenu::aboutToHide, this, [=] () {
        if (needHidden) {
            needHidden = false;
            q->setVisible(false);
        }
    });
}

CollectionTitleBarPrivate::~CollectionTitleBarPrivate()
{

}

void CollectionTitleBarPrivate::modifyTitleName()
{
    if (!renamable)
        return;

    if (q->isHidden())
        q->setHidden(false);

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

void CollectionTitleBarPrivate::showMenu()
{
    QAction *action = nullptr;
    // todo:调试确认（与标题栏隐藏存在冲突）
    if (adjustable) {
        action = new QAction(menu);
        action->setText(tr("Size"));
        menu->addAction(action);

        DMenu *subMenu = new DMenu(menu);
        action->setMenu(subMenu);

        action = new QAction(subMenu);
        action->setText(tr("Large"));
        subMenu->addAction(action);
        connect(action, &QAction::triggered, this, [=] () {
            emit q->sigRequestAdjustSizeMode(CollectionFrameSize::kLarge);
        });

        action = new QAction(subMenu);
        action->setText(tr("Small"));
        subMenu->addAction(action);
        connect(action, &QAction::triggered, this, [=] () {
            emit q->sigRequestAdjustSizeMode(CollectionFrameSize::kSmall);
        });
    }

//    if (renamable) {
//        action = new QAction(menu);
//        action->setText(tr("Rename"));
//        menu->addAction(action);
//        connect(action, &QAction::triggered, this, &CollectionTitleBarPrivate::modifyTitleName);
//    }

    if (closable) {
        menu->addSeparator();

        action = new QAction(menu);
        action->setText(tr("Delete"));
        menu->addAction(action);
        connect(action, &QAction::triggered, this, &CollectionTitleBarPrivate::sendRequestClose);
    }

    if (menu->actions().isEmpty())
        return;

    menu->exec(QCursor::pos());
    menu->clear();
}

void CollectionTitleBarPrivate::sendRequestClose()
{
    emit q->sigRequestClose(id);
}

CollectionTitleBar::CollectionTitleBar(const QString &uuid, QWidget *parent)
    : DBlurEffectWidget(parent)
    , d(new CollectionTitleBarPrivate(uuid, this))
{
    setObjectName("titleBar");
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(0, 31, 119, static_cast<int>(0.2*255)));

    d->nameWidget->installEventFilter(this);
}

CollectionTitleBar::~CollectionTitleBar()
{

}

bool CollectionTitleBar::setTitleBarVisible(const bool &visible)
{
    // todo 显隐逻辑待根据场景细化(需求：当鼠标移出集合时，隐藏该集合的标题栏)
    // 标题栏操作过程中（正在重命名、弹出菜单选项），移出鼠标是否隐藏标题栏？
    if (!visible && d->menu->isVisible()) {
        d->needHidden = true;
        return false;
    }

    d->needHidden = false;
    this->setVisible(visible);
    return true;
}

bool CollectionTitleBar::titleBarVisible() const
{
    return this->isVisible();
}

void CollectionTitleBar::setRenamable(const bool renamable)
{
    d->renamable = renamable;
}

bool CollectionTitleBar::renamable() const
{
    return d->renamable;
}

void CollectionTitleBar::setClosable(const bool closable)
{
    d->closable = closable;
}

bool CollectionTitleBar::closable() const
{
    return d->closable;
}

void CollectionTitleBar::setAdjustable(const bool adjustable)
{
    d->adjustable = adjustable;
}

bool CollectionTitleBar::adjustable() const
{
    return d->adjustable;
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
