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
#include "events/titlebareventcaller.h"

#include "dfm-base/base/device/deviceproxymanager.h"

#include <QAbstractButton>

using namespace dfmplugin_titlebar;

NavWidgetPrivate::NavWidgetPrivate(NavWidget *qq)
    : QObject(qq), q(qq)
{
}

void NavWidgetPrivate::updateBackForwardButtonsState()
{
    if (!curNavStack || curNavStack->size() <= 1) {
        navBackButton->setEnabled(false);
        navForwardButton->setEnabled(false);
    } else {
        if (curNavStack->isFirst()) {
            navBackButton->setEnabled(false);
        } else {
            navBackButton->setEnabled(true);
        }

        if (curNavStack->isLast()) {
            navForwardButton->setEnabled(false);
        } else {
            navForwardButton->setEnabled(true);
        }
    }
}

NavWidget::NavWidget(QWidget *parent)
    : QWidget(parent), d(new NavWidgetPrivate(this))
{
    initializeUi();
    initConnect();
}

void NavWidget::pushUrlToHistoryStack(const QUrl &url)
{
    if (!d->curNavStack)
        return;

    d->curNavStack->append(url);
    d->updateBackForwardButtonsState();
}

void NavWidget::back()
{
    QUrl &&url = d->curNavStack->back();

    if (!url.isEmpty()) {
        d->updateBackForwardButtonsState();
        TitleBarEventCaller::sendCd(this, url);
    }
}

void NavWidget::forward()
{
    QUrl &&url = d->curNavStack->forward();

    if (!url.isEmpty()) {
        d->updateBackForwardButtonsState();
        TitleBarEventCaller::sendCd(this, url);
    }
}

void NavWidget::onDevUnmounted(const QString &id, const QString &oldMpt)
{
    Q_UNUSED(id)

    auto mpt = QUrl::fromLocalFile(oldMpt);
    if (!mpt.isEmpty()) {
        for (auto stack : d->allNavStacks)
            stack->removeUrl(mpt);

        d->updateBackForwardButtonsState();
    }
}

void NavWidget::addHistroyStack()
{
    d->allNavStacks.append(std::shared_ptr<HistoryStack>(new HistoryStack(NavWidgetPrivate::kMaxStackCount)));
}

void NavWidget::moveNavStacks(int from, int to)
{
    d->allNavStacks.move(from, to);
}

void NavWidget::removeNavStackAt(int index)
{
    d->allNavStacks.removeAt(index);

    if (index < d->allNavStacks.count())
        d->curNavStack = d->allNavStacks.at(index);
    else
        d->curNavStack = d->allNavStacks.at(d->allNavStacks.count() - 1);

    if (!d->curNavStack)
        return;
    if (d->curNavStack->size() > 1)
        d->navBackButton->setEnabled(true);
    else
        d->navBackButton->setEnabled(false);

    if (d->curNavStack->isLast())
        d->navForwardButton->setEnabled(false);
    else
        d->navForwardButton->setEnabled(true);
}

void NavWidget::switchHistoryStack(const int index)
{
    d->curNavStack = d->allNavStacks.at(index);
    if (!d->curNavStack)
        return;
    d->updateBackForwardButtonsState();
}

void NavWidget::onUrlChanged(const QUrl &url)
{
    d->currentUrl = url;
    pushUrlToHistoryStack(url);
}

void NavWidget::onNewWindowOpended()
{
    addHistroyStack();
    pushUrlToHistoryStack(d->currentUrl);
}

void NavWidget::initializeUi()
{
    d->navBackButton = new DButtonBoxButton(QStyle::SP_ArrowBack);
    d->navBackButton->setDisabled(true);
    d->navBackButton->setFixedWidth(36);

    d->navForwardButton = new DButtonBoxButton(QStyle::SP_ArrowForward);
    d->navForwardButton->setDisabled(true);
    d->navForwardButton->setFixedWidth(36);

    d->buttonBox = new DButtonBox;
    d->hboxLayout = new QHBoxLayout;

    d->buttonBox->setButtonList({ d->navBackButton, d->navForwardButton }, false);
    this->setLayout(d->hboxLayout);
    d->hboxLayout->addWidget(d->buttonBox);

    d->hboxLayout->setSpacing(0);
    d->hboxLayout->setContentsMargins(0, 0, 0, 0);
}

void NavWidget::initConnect()
{
    DFMBASE_USE_NAMESPACE

    connect(d->navBackButton, &QAbstractButton::clicked, this, &NavWidget::back);
    connect(d->navForwardButton, &QAbstractButton::clicked, this, &NavWidget::forward);

    connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted, this, &NavWidget::onDevUnmounted);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, &NavWidget::onDevUnmounted);
    connect(DevProxyMng, &DeviceProxyManager::blockDevRemoved, this, &NavWidget::onDevUnmounted);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevRemoved, this, &NavWidget::onDevUnmounted);
}
