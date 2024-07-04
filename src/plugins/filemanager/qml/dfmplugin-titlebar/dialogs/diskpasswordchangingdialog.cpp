// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskpasswordchangingdialog.h"
#include "dpcwidget/dpcconfirmwidget.h"
#include "dpcwidget/dpcprogresswidget.h"
#include "dpcwidget/dpcresultwidget.h"

#include <DWindowManagerHelper>

#include <QStackedWidget>
#include <QCloseEvent>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_titlebar;

DiskPasswordChangingDialog::DiskPasswordChangingDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnect();
}

void DiskPasswordChangingDialog::initUI()
{
    setFixedSize(382, 286);
    setIcon(QIcon::fromTheme("dialog-warning"));

    switchPageWidget = new QStackedWidget(this);
    confirmWidget = new DPCConfirmWidget(this);
    progressWidget = new DPCProgressWidget(this);
    resultWidget = new DPCResultWidget(this);

    switchPageWidget->addWidget(confirmWidget);
    switchPageWidget->addWidget(progressWidget);
    switchPageWidget->addWidget(resultWidget);

    addContent(switchPageWidget);
    setContentsMargins(0, 0, 0, 0);
}

void DiskPasswordChangingDialog::initConnect()
{
    connect(confirmWidget, &DPCConfirmWidget::sigCloseDialog, this, &DiskPasswordChangingDialog::close);
    connect(confirmWidget, &DPCConfirmWidget::sigConfirmed, this, &DiskPasswordChangingDialog::onConfirmed);
    connect(progressWidget, &DPCProgressWidget::sigCompleted, this, &DiskPasswordChangingDialog::onChangeCompleted);
    connect(resultWidget, &DPCResultWidget::sigCloseDialog, this, &DiskPasswordChangingDialog::close);
}

void DiskPasswordChangingDialog::onConfirmed()
{
    DWindowManagerHelper::instance()->setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, false);
    progressWidget->start();
    switchPageWidget->setCurrentWidget(progressWidget);
}

void DiskPasswordChangingDialog::onChangeCompleted(bool success, const QString &msg)
{
    DWindowManagerHelper::instance()->setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, true);
    resultWidget->setResult(success, msg);
    switchPageWidget->setCurrentWidget(resultWidget);
}

void DiskPasswordChangingDialog::closeEvent(QCloseEvent *e)
{
    if (switchPageWidget && progressWidget == switchPageWidget->currentWidget())
        return e->ignore();

    DDialog::closeEvent(e);
}
