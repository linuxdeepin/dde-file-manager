/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "private/optionbuttonbox_p.h"
#include "optionbuttonbox.h"

DSB_FM_BEGIN_NAMESPACE

OptionButtonBoxPrivate::OptionButtonBoxPrivate(OptionButtonBox *parent)
    : QObject(parent)
    , q(parent)
{

}

QToolButton *OptionButtonBox::detailButton() const
{
    return d->detailButton;
}

void OptionButtonBox::setDetailButton(QToolButton *detailButton)
{
    if (!detailButton) return;

    if (!d->hBoxLayout->replaceWidget(detailButton, detailButton)->isEmpty()){

        if (d->detailButton) {
            delete d->detailButton;
            d->detailButton = nullptr;
        }

        if (!d->detailButton)
            d->detailButton = detailButton;

        if (d->listViewButton->icon().isNull())
            d->detailButton->setIcon(QIcon::fromTheme("dfrightview_detail"));

        d->detailButton->setCheckable(true);
        d->detailButton->setFocusPolicy(Qt::NoFocus);
        d->detailButton->setIconSize({16,16});
        d->detailButton->setFixedSize(36, 36);
    }
}

QToolButton *OptionButtonBox::listViewButton() const
{
    return d->listViewButton;
}

void OptionButtonBox::setListViewButton(QToolButton *listViewButton)
{
    if (!listViewButton) return;

    if (!d->hBoxLayout->replaceWidget(listViewButton, listViewButton)->isEmpty()){

        if (d->listViewButton) {
            delete d->listViewButton;
            d->listViewButton = nullptr;
        }

        if (!listViewButton)
            d->listViewButton = listViewButton;

        if (d->listViewButton->icon().isNull())
            d->listViewButton->setIcon(QIcon::fromTheme("dfviewlist_details"));

        d->listViewButton->setCheckable(true);
        d->listViewButton->setFocusPolicy(Qt::NoFocus);
        d->listViewButton->setIconSize({16,16});
        d->listViewButton->setFixedSize(36, 36);
    }
}

OptionButtonBox::OptionButtonBox(QWidget *parent)
    : QWidget (parent)
    , d(new OptionButtonBoxPrivate(this))
{
    if (!d->iconViewButton)
        d->iconViewButton = new QToolButton;
    d->iconViewButton->setCheckable(true);
    d->iconViewButton->setFocusPolicy(Qt::NoFocus);
    d->iconViewButton->setIcon(QIcon::fromTheme("dfviewlist_icons"));
    d->iconViewButton->setIconSize({16,16});
    d->iconViewButton->setFixedSize(36, 36);

    if (!d->listViewButton)
        d->listViewButton = new QToolButton;
    d->listViewButton->setCheckable(true);
    d->listViewButton->setFocusPolicy(Qt::NoFocus);
    d->listViewButton->setIcon(QIcon::fromTheme("dfviewlist_details"));
    d->listViewButton->setIconSize({16,16});
    d->listViewButton->setFixedSize(36, 36);

    if (!d->detailButton)
        d->detailButton = new QToolButton;
    d->detailButton->setCheckable(true);
    d->detailButton->setFocusPolicy(Qt::NoFocus);
    d->detailButton->setIcon(QIcon::fromTheme("dfrightview_detail"));
    d->detailButton->setIconSize({16,16});
    d->detailButton->setFixedSize(36, 36);
    d->detailButton->show();

    if (!d->hBoxLayout)
        d->hBoxLayout = new QHBoxLayout;
    d->hBoxLayout->addSpacing(15);
    d->hBoxLayout->addWidget(d->iconViewButton);
    d->hBoxLayout->addWidget(d->listViewButton);
    d->hBoxLayout->addWidget(d->detailButton);
    d->hBoxLayout->setContentsMargins(0,0,4,0);
    d->hBoxLayout->setSpacing(18);

    setLayout(d->hBoxLayout);
}

QToolButton *OptionButtonBox::iconViewButton() const
{
    return d->iconViewButton;
}

void OptionButtonBox::setIconViewButton(QToolButton *iconViewButton)
{
    if (!iconViewButton) return;

    if (!d->hBoxLayout->replaceWidget(iconViewButton, iconViewButton)->isEmpty()){

        if (d->iconViewButton) {
            delete d->iconViewButton;
            d->iconViewButton = nullptr;
        }

        if (!iconViewButton)
            d->iconViewButton = iconViewButton;

        if (d->iconViewButton->icon().isNull())
            d->iconViewButton->setIcon(QIcon::fromTheme("dfviewlist_icons"));

        d->iconViewButton->setCheckable(true);
        d->iconViewButton->setFocusPolicy(Qt::NoFocus);
        d->iconViewButton->setIconSize({16,16});
        d->iconViewButton->setFixedSize(36, 36);
    }
}

DSB_FM_END_NAMESPACE
