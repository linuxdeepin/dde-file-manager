/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "views/private/navwidget_p.h"
#include "views/navwidget.h"

DPTITLEBAR_USE_NAMESPACE

void NavWidgetPrivate::doButtonClicked()
{
    if (!sender() || listIdx == -1) return;

    if (sender() == navBackButton) {
        if (0 == listIdx) {
            Q_EMIT q->releaseUrl(urlCacheList[listIdx]);
            return;   // 头节点
        }
        Q_EMIT q->releaseUrl(urlCacheList[--listIdx]);
        return;
    }

    if (sender() == navForwardButton) {
        if (urlCacheList.size() - 1 == listIdx) {
            Q_EMIT q->releaseUrl(urlCacheList[listIdx]);   // 尾节点
            return;
        }
        q->releaseUrl(urlCacheList[++listIdx]);
        return;
    }
}

NavWidgetPrivate::NavWidgetPrivate(NavWidget *qq)
    : QObject(qq), q(qq)
{
}

NavWidget::NavWidget(QWidget *parent)
    : QWidget(parent), d(new NavWidgetPrivate(this))
{
    if (!d->navBackButton)
        d->navBackButton = new DButtonBoxButton(QStyle::SP_ArrowBack);
    if (!d->navForwardButton)
        d->navForwardButton = new DButtonBoxButton(QStyle::SP_ArrowForward);
    if (!d->buttonBox)
        d->buttonBox = new DButtonBox;
    if (!d->hboxLayout)
        d->hboxLayout = new QHBoxLayout;

    d->buttonBox->setButtonList({ d->navBackButton, d->navForwardButton }, false);
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

    d->buttonBox->setButtonList({ d->navBackButton, d->navForwardButton }, false);
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

    if (!d->navForwardButton)
        d->navForwardButton = navForwardButton;

    if (d->navBackButton->icon().isNull())
        d->navBackButton->setIcon(QStyle::SP_ArrowForward);

    d->buttonBox->setButtonList({ d->navBackButton, d->navForwardButton }, false);
}

void NavWidget::appendUrl(const QUrl &url)
{
    //始终保持指针指向最后

    if (d->listIdx >= 0
        && d->listIdx < d->urlCacheList.size()) {
        if (d->urlCacheList[d->listIdx] == url) {
            return;   //略过当前目录的重复点击
        }
    }

    d->urlCacheList.append(url);
    d->listIdx = d->urlCacheList.size() - 1;
}
