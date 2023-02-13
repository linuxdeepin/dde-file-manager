// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskpasswordchangingdialog.h"
#include "dpcwidget/dpcconfirmwidget.h"
#include "dpcwidget/dpcprogresswidget.h"
#include "dpcwidget/dpcresultwidget.h"

#include <DWindowManagerHelper>

#include <QStackedWidget>

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

void DiskPasswordChangingDialog::displayNextPage()
{
    Q_ASSERT(switchPageWidget);

    int nIndex = switchPageWidget->currentIndex();
    int nCount = switchPageWidget->count();

    if (nIndex < nCount - 1) {
        switchPageWidget->setCurrentIndex(++nIndex);
    }
}

void DiskPasswordChangingDialog::onConfirmed()
{
    DWindowManagerHelper::instance()->setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, false);
    progressWidget->start();
    displayNextPage();
}

void DiskPasswordChangingDialog::onChangeCompleted(bool success, const QString &msg)
{
    DWindowManagerHelper::instance()->setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, true);
    resultWidget->setResult(success, msg);
    displayNextPage();
}
