// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskpwdchangedialog.h"
#include "pwdconfirmwidget.h"
#include "changeresultwidget.h"
#include "progresswidget.h"

#include <DWindowManagerHelper>

#include <QStackedWidget>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

DiskPwdChangeDialog::DiskPwdChangeDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnect();
}

void DiskPwdChangeDialog::initUI()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(382, 286);
    setIcon(QIcon::fromTheme("dialog-warning"));

    m_switchPageWidget = new QStackedWidget(this);
    m_confirmWidget = new PwdConfirmWidget(this);
    m_progressWidget = new ProgressWidget(this);
    m_resultWidget = new ChangeResultWidget(this);

    m_switchPageWidget->addWidget(m_confirmWidget);
    m_switchPageWidget->addWidget(m_progressWidget);
    m_switchPageWidget->addWidget(m_resultWidget);

    addContent(m_switchPageWidget);
    setContentsMargins(0, 0, 0, 0);
}

void DiskPwdChangeDialog::initConnect()
{
    connect(m_confirmWidget, &PwdConfirmWidget::sigClosed, this, &DiskPwdChangeDialog::close);
    connect(m_confirmWidget, &PwdConfirmWidget::sigConfirmed, this, &DiskPwdChangeDialog::pwdConfirmed);
    connect(m_progressWidget, &ProgressWidget::sigChangeFinished, this, &DiskPwdChangeDialog::pwdChangeFinished);
    connect(m_resultWidget, &ChangeResultWidget::sigClosed, this, &DiskPwdChangeDialog::close);
}

void DiskPwdChangeDialog::displayNextPage()
{
    Q_ASSERT(m_switchPageWidget);

    int nIndex = m_switchPageWidget->currentIndex();
    int nCount = m_switchPageWidget->count();

    if (nIndex < nCount - 1) {
        m_switchPageWidget->setCurrentIndex(++nIndex);
    }
}

void DiskPwdChangeDialog::pwdConfirmed()
{
    DWindowManagerHelper::instance()->setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, false);
    m_progressWidget->start();
    displayNextPage();
}

void DiskPwdChangeDialog::pwdChangeFinished(bool success, const QString &msg)
{
    DWindowManagerHelper::instance()->setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, true);
    m_resultWidget->setResult(success, msg);
    displayNextPage();
}
