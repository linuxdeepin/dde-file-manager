// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/navwidget_p.h"
#include "views/navwidget.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-framework/event/event.h>

#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

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
        if (curNavStack->isFirst() || !curNavStack->backIsExist()) {
            navBackButton->setEnabled(false);
        } else {
            navBackButton->setEnabled(true);
        }

        if (curNavStack->isLast() || !curNavStack->forwardIsExist()) {
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

void NavWidget::removeUrlFromHistoryStack(const QUrl &url)
{
    if (!d->curNavStack)
        return;
    d->curNavStack->removeUrl(url);
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
    d->navForwardButton = new DButtonBoxButton(QStyle::SP_ArrowForward);
    d->navForwardButton->setDisabled(true);
    d->buttonBox = new DButtonBox;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(d->navBackButton), AcName::kAcComputerTitleBarBackBtn);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(d->navForwardButton), AcName::kAcComputerTitleBarForwardBtn);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(d->buttonBox), AcName::kAcComputerTitleBarBtnBox);
#endif
    d->hboxLayout = new QHBoxLayout;

    d->buttonBox->setButtonList({ d->navBackButton, d->navForwardButton }, false);
    this->setLayout(d->hboxLayout);
    d->hboxLayout->addWidget(d->buttonBox);

    d->hboxLayout->setSpacing(0);
    d->hboxLayout->setContentsMargins(0, 0, 0, 0);

    changeSizeMode();
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

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &NavWidget::changeSizeMode);
#endif
}

void NavWidget::changeSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    QSize smallSize(24, 24);
    QSize normalSize(36, 36);
    d->navBackButton->setFixedSize(DSizeModeHelper::element(smallSize, normalSize));
    d->navForwardButton->setFixedSize(DSizeModeHelper::element(smallSize, normalSize));
#else
    QSize normalSize(36, 36);
    d->navBackButton->setFixedSize(normalSize);
    d->navForwardButton->setFixedSize(normalSize);
#endif
}
