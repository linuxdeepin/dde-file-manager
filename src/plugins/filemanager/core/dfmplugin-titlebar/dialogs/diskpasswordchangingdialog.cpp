/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
