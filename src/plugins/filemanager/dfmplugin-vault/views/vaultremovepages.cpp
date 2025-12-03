// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovepages.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/servicemanager.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/vaultconfig.h"
#include "removevaultview/vaultremoveprogressview.h"
#include "removevaultview/vaultremovebypasswordview.h"
#include "removevaultview/vaultremovebyrecoverykeyview.h"
#include "removevaultview/vaultremovebynonewidget.h"

#include "plugins/common/dfmplugin-utils/reportlog/datas/vaultreportdata.h"

#include <dfm-framework/event/event.h>

#include <dfm-base/utils/windowutils.h>

#include <DLabel>

#include <QFrame>
#include <QRegularExpressionValidator>
#include <QStackedWidget>
#include <QAbstractButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWindow>

Q_DECLARE_METATYPE(const char *)

constexpr int kKeyVerifyDeleteBtn = 1;
constexpr int kPassWordDeleteBtn = 2;

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultRemovePages::VaultRemovePages(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnect();
}

void VaultRemovePages::pageSelect(RemoveWidgetType type)
{
    fmDebug() << "Vault: Selecting remove page type:" << static_cast<int>(type);
    switch (type) {
    case kPasswordWidget: {
        fmDebug() << "Vault: Showing password widget";
        showPasswordWidget();
    } break;
    case kRecoveryKeyWidget: {
        fmDebug() << "Vault: Showing recovery key widget";
        showRecoveryKeyWidget();
    } break;
    case kRemoveProgressWidget: {
        fmDebug() << "Vault: Showing remove progress widget";
        showRemoveProgressWidget();
    } break;
    case kNoneWidget: {
        fmDebug() << "Vault: Showing none widget";
        showNodeWidget();
    } break;
    default:
        fmDebug() << "Vault: Unknown remove widget type:" << static_cast<int>(type);
        break;
    }
}

void VaultRemovePages::initConnect()
{
    connect(this, &VaultRemovePages::buttonClicked, this, &VaultRemovePages::onButtonClicked);
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
        addButton(btns[1], true, ButtonType::ButtonWarning);
    }
    connect(passwordView, &VaultRemoveByPasswordView::signalJump, this, &VaultRemovePages::pageSelect);
    connect(passwordView, &VaultRemoveByPasswordView::sigCloseDialog, this, &VaultRemovePages::close);
}

void VaultRemovePages::showRecoveryKeyWidget()
{
    clearContents(true);
    clearButtons();

    recoverykeyView = new VaultRemoveByRecoverykeyView(this);
    setTitle(recoverykeyView->titleText());
    addContent(recoverykeyView);
    QStringList btns = recoverykeyView->btnText();
    if (btns.size() > 1) {
        addButton(btns[0], false);
        addButton(btns[1], true, ButtonType::ButtonWarning);
    }
    connect(recoverykeyView, &VaultRemoveByRecoverykeyView::signalJump, this, &VaultRemovePages::pageSelect);
    connect(recoverykeyView, &VaultRemoveByRecoverykeyView::sigCloseDialog, this, &VaultRemovePages::close);
}

void VaultRemovePages::showRemoveProgressWidget()
{
    clearContents(true);
    clearButtons();

    progressView = new VaultRemoveProgressView(this);
    setTitle(progressView->titleText());
    addContent(progressView);
    QStringList btns = progressView->btnText();
    if (btns.size() > 0) {
        addButton(btns[0], true, ButtonType::ButtonRecommend);
        getButton(0)->setFixedWidth(200);
    }

    connect(progressView, &VaultRemoveProgressView::sigCloseDialog, this, &VaultRemovePages::close);
    connect(progressView, &VaultRemoveProgressView::setBtnEnable, this, &VaultRemovePages::setBtnEnable);

    progressView->removeVault(kVaultBasePath);
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
        addButton(btns[1], true, ButtonType::ButtonWarning);
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
        recoverykeyView->buttonClicked(index, text);
    } else if (getContent(0) == progressView) {
        progressView->buttonClicked(index, text);
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

    setIcon(QIcon::fromTheme("safebox"));
    this->setFixedWidth(396);
    setOnButtonClickedClose(false);
}
