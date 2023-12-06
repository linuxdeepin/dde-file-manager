// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovepages.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaulthelper.h"
#include "utils/policy/policymanager.h"
#include "utils/pathmanager.h"
#include "utils/servicemanager.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/vaultconfig.h"
#include "removevaultview/vaultremoveprogressview.h"
#include "removevaultview/vaultremovebypasswordview.h"
#include "removevaultview/vaultremovebyrecoverykeyview.h"
#include "removevaultview/vaultremovebytpmpinwidget.h"
#include "removevaultview/vaultremovebynonewidget.h"

#include "plugins/common/dfmplugin-utils/reportlog/datas/vaultreportdata.h"

#include <dfm-framework/event/event.h>

#include <dfm-base/utils/windowutils.h>

#include <DLabel>

#include <QFrame>
#include <QRegExpValidator>
#include <QStackedWidget>
#include <QAbstractButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWindow>

Q_DECLARE_METATYPE(const char *)

using namespace PolkitQt1;
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;
using namespace dfmplugin_utils;

VaultRemovePages::VaultRemovePages(QWidget *parent)
    : VaultPageBase(parent)
{
    initUI();
    initConnect();
}

void VaultRemovePages::pageSelect(RemoveWidgetType type)
{
    switch (type) {
    case kPasswordWidget: {
        showPasswordWidget();
    } break;
    case kRecoveryKeyWidget: {
        showRecoveryKeyWidget();
    } break;
    case kRemoveProgressWidget: {
        showRemoveProgressWidget();
    } break;
    case kTpmPinWidget: {
        showTpmPinWidget();
    } break;
    case kNoneWidget: {
        showNodeWidget();
    } break;
    default:
        break;
    }
}

void VaultRemovePages::initConnect()
{
    connect(this, &VaultRemovePages::buttonClicked, this, &VaultRemovePages::onButtonClicked);
}

void VaultRemovePages::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kDeleteVaultPage);
    VaultPageBase::showEvent(event);
}

void VaultRemovePages::showPasswordWidget()
{
    clearContents(true);
    clearButtons();

    passwordView = new VaultRemoveByPasswordView(this);
    setTitle(passwordView->titleText());
    addContent(passwordView);
    QStringList btns = passwordView->btnText();
    if (btns.size() > 1) {
        addButton(btns[0], false);
        addButton(btns[1], true, ButtonType::ButtonRecommend);
    }
    connect(passwordView, &VaultRemoveByPasswordView::signalJump, this, &VaultRemovePages::pageSelect);
    connect(passwordView, &VaultRemoveByPasswordView::sigCloseDialog, this, &VaultRemovePages::close);
}

void VaultRemovePages::showRecoveryKeyWidget()
{
    recoverykeyView = new VaultRemoveByRecoverykeyView(this);
}

void VaultRemovePages::showRemoveProgressWidget()
{
    clearContents(true);
    if (getButtons().size() > 1) {
        getButton(0)->setVisible(false);
        getButton(1)->setVisible(false);
    }
    clearButtons();

    progressView = new VaultRemoveProgressView(this);
    setTitle(progressView->titleText());
    addContent(progressView);
    QStringList btns = progressView->btnText();
    if (btns.size() > 0)
        addButton(btns[0], true, ButtonType::ButtonRecommend);

    connect(progressView, &VaultRemoveProgressView::sigCloseDialog, this, &VaultRemovePages::close);
    connect(progressView, &VaultRemoveProgressView::setBtnEnable, this, &VaultRemovePages::setBtnEnable);

    progressView->removeVault(kVaultBasePath);
}

void VaultRemovePages::showTpmPinWidget()
{
    clearContents(true);
    clearButtons();

    tpmPinWidget = new VaultRemoveByTpmPinWidget(this);
    setTitle(tpmPinWidget->titleText());
    addContent(tpmPinWidget);
    QStringList btns = tpmPinWidget->btnText();
    if (btns.size() > 1) {
        addButton(btns[0], false);
        addButton(btns[1], true, ButtonType::ButtonRecommend);
    }

    connect(tpmPinWidget, &VaultRemoveByTpmPinWidget::closeDialog, this, &VaultRemovePages::close);
    connect(tpmPinWidget, &VaultRemoveByTpmPinWidget::jumpPage, this, &VaultRemovePages::pageSelect);
}

void VaultRemovePages::showNodeWidget()
{
    clearContents(true);
    clearButtons();

    noneWidget = new VaultRemoveByNoneWidget(this);
    setTitle(noneWidget->titleText());
    addContent(noneWidget);
    QStringList btns = noneWidget->btnText();
    if (btns.size() > 1) {
        addButton(btns[0], false);
        addButton(btns[1], true, ButtonType::ButtonRecommend);
    }

    connect(noneWidget, &VaultRemoveByNoneWidget::closeDialog, this, &VaultRemovePages::close);
    connect(noneWidget, &VaultRemoveByNoneWidget::jumpPage, this, &VaultRemovePages::pageSelect);
}

void VaultRemovePages::onButtonClicked(int index, const QString &text)
{
    if (!getContent(0))
        return;

    if (getContent(0) == passwordView) {
        passwordView->buttonClicked(index, text);
    } else if (getContent(0) == recoverykeyView) {

    } else if (getContent(0) == progressView) {
        progressView->buttonClicked(index, text);
    } else if (getContent(0) == tpmPinWidget) {
        tpmPinWidget->buttonClicked(index, text);
    } else if (getContent(0) == noneWidget) {
        noneWidget->buttonClicked(index, text);
    }
}

void VaultRemovePages::setBtnEnable(int index, bool enable)
{
    if (getButton(index)) {
        getButton(index)->setEnabled(enable);
    }
}

void VaultRemovePages::initUI()
{
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    if (dfmbase::WindowUtils::isWayLand()) {
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setIcon(QIcon(":/icons/deepin/builtin/icons/dfm_vault_32px.svg"));
    this->setFixedWidth(396);
    setOnButtonClickedClose(false);
}
