// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarcontainment.h"
#include "models/quickcrumbmodel.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"
#include "events/titlebareventcaller.h"

#include <QDebug>

DFMBASE_USE_NAMESPACE

namespace dfmplugin_titlebar {

TitlebarContainment::TitlebarContainment(QObject *parent)
    : dfmgui::Containment(parent), model(new QuickCrumbModel)
{
    connect(this, &Applet::currentUrlChanged, this, &TitlebarContainment::onUrlChanged);
}

QuickCrumbModel *TitlebarContainment::crumbModel() const
{
    return model;
}

void TitlebarContainment::onUrlChanged(const QUrl &url)
{
    if (rootObject()) {
        TitleBarEventCaller::sendCd(this, url);
    }

    updateController(url);

    if (crumbController) {
        crumbController->crumbUrlChangedBehavior(url);
    }
}

dfmbase::Global::ViewMode TitlebarContainment::viewMode() const
{
    return internalViewMode;
}

void TitlebarContainment::setViewMode(dfmbase::Global::ViewMode mode)
{
    if (mode != internalViewMode) {
        internalViewMode = mode;
        Q_EMIT viewModeChanged();
    }
}

bool TitlebarContainment::showDetail() const
{
    return showDetailInfo;
}

void TitlebarContainment::setShowDetail(bool b)
{
    if (b != showDetailInfo) {
        showDetailInfo = b;
        Q_EMIT showDetailChanged();

        if (rootObject()) {
            TitleBarEventCaller::sendDetailViewState(this, showDetailInfo);
        }
    }
}

void TitlebarContainment::updateController(const QUrl &url)
{
    if (!crumbController || !crumbController->isSupportedScheme(url.scheme())) {
        if (crumbController) {
            crumbController->deleteLater();
        }
        crumbController = CrumbManager::instance()->createControllerByUrl(url);
        // Not found? Then nothing here...
        if (!crumbController) {
            fmWarning() << "Unsupported url / scheme: " << url;
            // always has default controller
            crumbController = new CrumbInterface;
        }
        crumbController->setParent(this);
        // QObject::connect(crumbController, &CrumbInterface::hideAddressBar, q, &CrumbBar::hideAddressBar);
        // QObject::connect(crumbController, &CrumbInterface::keepAddressBar, q, &CrumbBar::onKeepAddressBar);
        QObject::connect(crumbController, &CrumbInterface::hideAddrAndUpdateCrumbs, this, &TitlebarContainment::onHideAddrAndUpdateCrumbs);
    }
}

void TitlebarContainment::onHideAddrAndUpdateCrumbs(const QUrl &url)
{
    if (!crumbController) {
        fmWarning("No controller found when trying to call DFMCrumbBar::updateCrumbs() !!!");
        fmDebug() << "updateCrumbs (no controller) : " << url;
        return;
    }

    setCurrentUrl(url);

    QList<CrumbData> &&crumbDataList = crumbController->seprateUrl(url);
    model->setCurrentUrl(url, crumbDataList);
}

}
