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
#include "dfmvaultsetuppages.h"
#include "dfmvaultunlockpages.h"
#include "dfmvaultrecoverykeypages.h"

#include <QVBoxLayout>

DFM_BEGIN_NAMESPACE

FallbackDispatcher::FallbackDispatcher(QWidget *parent)
    : DFMVaultContentInterface(parent)
    , m_label(new QLabel(this))
{
    QVBoxLayout layout(this);
    layout.addWidget(m_label);
}

QPair<DUrl, bool> FallbackDispatcher::requireRedirect(VaultController::VaultState state)
{
    // gongheng测试
    return {VaultController::makeVaultUrl(), true};

//    switch (state) {
//    case VaultController::Unlocked:
//        return {VaultController::makeVaultUrl(), true};
//    case VaultController::Encrypted:
//        return {VaultController::makeVaultUrl("/", "unlock"), true};
//    default:
//        return {VaultController::makeVaultUrl("/", "setup"), true};
//    }
}

// --------------------------------------------------------

DFMVaultView::DFMVaultView(QWidget *parent)
    : QWidget (parent)
    , m_rootUrl(DFMVAULT_SCHEME)
    , m_containerLayout(new QVBoxLayout(this))
    , m_contentWidget(nullptr)
{
    registerContentWidget("setup", new DFMVaultSetupPages(this));
    registerContentWidget("unlock", new DFMVaultUnlockPages(this));
    registerContentWidget("recovery_key", new DFMVaultRecoveryKeyPages(this));
    registerContentWidget("_fallback_", new FallbackDispatcher(this));
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
    m_rootUrl = url;

    VaultController::VaultState state = VaultController::getVaultController()->state();
    QString contentKey = url.host();

    if (!m_contentMap.contains(contentKey)) {
        contentKey = "_fallback_";
    }

    DFMVaultContentInterface * contentWidget = m_contentMap.value(contentKey);
    if (!contentWidget) return false;

    QPair<DUrl, bool> requireRedirect = contentWidget->requireRedirect(state);
    if (requireRedirect.second) {
        cd(requireRedirect.first);
        return false;
    }

    contentWidget->setRootUrl(m_rootUrl);
    QWidget * oldWidget = replaceContainerWidget(contentWidget);
    if (oldWidget) oldWidget->setVisible(false);

    return true;
}

/*!
 * \brief replace the content widget with a new given \a widget
 *
 * It's the caller's response to manage the ownership of the given widget.
 *
 * \return the old widget we replaced, or null if we are not replace any widget.
 */
QWidget *DFMVaultView::replaceContainerWidget(QWidget *widget)
{
    if (!widget) return nullptr;
    if (widget == m_contentWidget) return nullptr;

    QWidget * oldWidget = nullptr;

    if (m_contentWidget) {
        m_containerLayout->removeWidget(m_contentWidget);
        oldWidget = m_contentWidget;
    }

    m_contentWidget = widget;
    m_contentWidget->setVisible(true);
    m_containerLayout->addWidget(widget);

    return oldWidget;
}

bool DFMVaultView::cd(const DUrl &url)
{
    DFileManagerWindow* w = qobject_cast<DFileManagerWindow*>(WindowManager::getWindowById(WindowManager::getWindowId(this)));

    return w && w->cd(url);
}

void DFMVaultView::registerContentWidget(const QString &name, DFMVaultContentInterface *widget)
{
    widget->setVisible(false);
    m_contentMap.insert(name, widget);
    connect(widget, &DFMVaultContentInterface::requestRedirect, this, &DFMVaultView::cd);
}

DFM_END_NAMESPACE
