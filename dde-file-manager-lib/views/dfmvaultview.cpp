/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmvaultview.h"

#include "dfilemanagerwindow.h"
#include "windowmanager.h"
#include "controllers/vaultcontroller.h"

#include <QLabel>
#include <QVBoxLayout>

DFM_BEGIN_NAMESPACE

DFMVaultView::DFMVaultView(QWidget *parent)
    : QWidget (parent)
    , m_rootUrl(DFMVAULT_SCHEME)
    , m_containerLayout(new QVBoxLayout(this))
    , m_contentWidget(nullptr)
{

}

DFMVaultView::~DFMVaultView()
{

}

QWidget *DFMVaultView::widget() const
{
    return const_cast<DFMVaultView*>(this);
}

DUrl DFMVaultView::rootUrl() const
{
    return m_rootUrl;
}

bool DFMVaultView::setRootUrl(const DUrl &url)
{
    static QStringList availableViewStates = {
        "unlock",
        "password_recovery",
        "setup",
        "import"
    };

    m_rootUrl = url;

    VaultController::VaultState state = VaultController::state();

    if (!availableViewStates.contains(url.host())) {
        switch (state) {
        case VaultController::Unlocked:
            cd(VaultController::makeVaultUrl());
            return false;
        case VaultController::Encrypted:
            cd(VaultController::makeVaultUrl("/", "unlock"));
            return false;
        default:
            cd(VaultController::makeVaultUrl("/", "setup"));
            return false;
        }
    }

    setContainerWidget(new QLabel(url.toString()));

    return true;
}

void DFMVaultView::setContainerWidget(QWidget *widget)
{
    if (!widget) return;

    if (m_contentWidget) {
        m_containerLayout->removeWidget(m_contentWidget);
        QWidget * oldWidget = m_contentWidget;
        oldWidget->deleteLater();
    }

    m_contentWidget = widget;
    m_containerLayout->addWidget(widget);
}

bool DFMVaultView::cd(const DUrl &url)
{
    DFileManagerWindow* w = qobject_cast<DFileManagerWindow*>(WindowManager::getWindowById(WindowManager::getWindowId(this)));

    return w && w->cd(url);
}

DFM_END_NAMESPACE
