/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "closealldialog.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/universalutils.h"

#include <DLabel>

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE
CloseAllDialog::CloseAllDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::NoFocus);
    //    AC_SET_OBJECT_NAME(this, AC_CLOSE_ALL_DLG_INDICATOR);
    //    AC_SET_ACCESSIBLE_NAME(this, AC_CLOSE_ALL_DLG_INDICATOR);

    initUI();
    initConnect();
}

CloseAllDialog::~CloseAllDialog()
{
}

void CloseAllDialog::initUI()
{
    resize(QSize(400, 50));

    messageLabel = new DLabel(this);
    //    AC_SET_OBJECT_NAME(messageLabel, AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL);
    //    AC_SET_ACCESSIBLE_NAME(messageLabel, AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL);

    closeButton = new QPushButton(tr("Close all"), this);
    //    AC_SET_OBJECT_NAME(closeButton, AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON);
    //    AC_SET_ACCESSIBLE_NAME(closeButton, AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(messageLabel, Qt::AlignCenter);
    mainLayout->addSpacing(50);
    mainLayout->addWidget(closeButton, Qt::AlignRight);
    mainLayout->setContentsMargins(25, 5, 25, 5);
    setLayout(mainLayout);

    setTotalMessage(0, 0);
}

void CloseAllDialog::initConnect()
{
    connect(closeButton, &QPushButton::clicked, this, &CloseAllDialog::allClosed);
}

void CloseAllDialog::setTotalMessage(qint64 size, int count)
{
    QString message = tr("Total size: %1, %2 files").arg(FileUtils::formatSize(size), QString::number(count));
    messageLabel->setText(message);
}

void CloseAllDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        return;
    }
    QDialog::keyPressEvent(event);
}

void CloseAllDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    QRect screenGeometry = qApp->desktop()->availableGeometry();

    int geometryHeight = screenGeometry.height() - UniversalUtils::dockHeight();
    int geometryWidth = screenGeometry.width();

    move((geometryWidth - width()) / 2, geometryHeight - height());

    return DAbstractDialog::showEvent(event);
}
