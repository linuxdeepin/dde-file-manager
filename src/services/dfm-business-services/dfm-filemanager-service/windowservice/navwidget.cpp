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
#include "private/navwidget_p.h"
#include "navwidget.h"

DSB_FM_BEGIN_NAMESPACE

namespace GlobalPrivate {
    static int listIdx = -1;
    static QList<QUrl> urlCacheList{};
} //namespace GlobalPrivate

void NavWidgetPrivate::doButtonClicked()
{
    if (!sender() || GlobalPrivate::listIdx == -1) return;

    if (sender() == navBackButton) {
        if (0 == GlobalPrivate::listIdx) {
            Q_EMIT q_ptr->releaseUrl(GlobalPrivate::urlCacheList[GlobalPrivate::listIdx]);
            return;  //头节点
        }
        q_ptr->releaseUrl(GlobalPrivate::urlCacheList[-- GlobalPrivate::listIdx]);
        return;
    }

    if (sender() == navForwardButton) {
        if (GlobalPrivate::urlCacheList.size() - 1 == GlobalPrivate::listIdx ) {
            Q_EMIT q_ptr->releaseUrl(GlobalPrivate::urlCacheList[GlobalPrivate::listIdx]); //尾节点
            return;
        }
        q_ptr->releaseUrl(GlobalPrivate::urlCacheList[++ GlobalPrivate::listIdx]);
        return;
    }
}

NavWidgetPrivate::NavWidgetPrivate(NavWidget *qq)
    : QObject (qq)
    , q_ptr(qq)
{

}

NavWidget::NavWidget(QWidget *parent)
    : QWidget(parent)
    , d(new NavWidgetPrivate(this))
{
    if (!d->navBackButton)
        d->navBackButton = new DButtonBoxButton(QStyle::SP_ArrowBack);
    if (!d->navForwardButton)
        d->navForwardButton = new DButtonBoxButton(QStyle::SP_ArrowForward);
    if (!d->buttonBox)
        d->buttonBox = new DButtonBox;
    if (!d->hboxLayout)
        d->hboxLayout = new QHBoxLayout;

    d->buttonBox->setButtonList({d->navBackButton,d->navForwardButton},false);
    this->setLayout(d->hboxLayout);
    d->hboxLayout->addWidget(d->buttonBox);

    d->hboxLayout->setSpacing(0);
    d->hboxLayout->setContentsMargins(0, 0, 0, 0);

    QObject::connect(d->navBackButton, &DButtonBoxButton::clicked,
                     d, &NavWidgetPrivate::doButtonClicked,
                     Qt::UniqueConnection);

    QObject::connect(d->navForwardButton, &DButtonBoxButton::clicked,
                     d, &NavWidgetPrivate::doButtonClicked,
                     Qt::UniqueConnection);
}

DButtonBoxButton *NavWidget::navBackButton() const
{
    return d->navBackButton;
}

void NavWidget::setNavBackButton(DButtonBoxButton *navBackButton)
{
    if (!d->navBackButton) return;

    if (d->navBackButton) {
        delete d->navBackButton;
        d->navBackButton = nullptr;
    }

    if (!d->navBackButton)
        d->navBackButton = navBackButton;

    if (d->navBackButton->icon().isNull())
        d->navBackButton->setIcon(QStyle::SP_ArrowBack);

    d->buttonBox->setButtonList({d->navBackButton,d->navForwardButton},false);
}

DButtonBoxButton *NavWidget::navForwardButton() const
{
    return d->navForwardButton;
}

void NavWidget::setNavForwardButton(DButtonBoxButton *navForwardButton)
{
    if (!d->navForwardButton) return;

    if (d->navForwardButton) {
        delete d->navForwardButton;
        d->navForwardButton = nullptr;
    }

    if(!d->navForwardButton)
        d->navForwardButton = navForwardButton;

    if (d->navBackButton->icon().isNull())
        d->navBackButton->setIcon(QStyle::SP_ArrowForward);

    d->buttonBox->setButtonList({d->navBackButton,d->navForwardButton},false);
}

void NavWidget::appendUrl(const QUrl &url)
{
    if (GlobalPrivate::listIdx != -1) {
        if(GlobalPrivate::urlCacheList[GlobalPrivate::listIdx] == url) return;//略过当前目录的重复点击
    }

    GlobalPrivate::urlCacheList.append(url);
    GlobalPrivate::listIdx = GlobalPrivate::urlCacheList.size() - 1; //始终保持指针指向最后
}

DSB_FM_END_NAMESPACE
