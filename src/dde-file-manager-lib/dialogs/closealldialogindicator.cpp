// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "closealldialogindicator.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QKeyEvent>
#include <QDBusInterface>
#include "shutil/fileutils.h"
#include "accessibility/ac-lib-file-manager.h"

CloseAllDialogIndicator::CloseAllDialogIndicator(QWidget *parent) : DAbstractDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::NoFocus);
    AC_SET_OBJECT_NAME(this, AC_CLOSE_ALL_DLG_INDICATOR);
    AC_SET_ACCESSIBLE_NAME(this, AC_CLOSE_ALL_DLG_INDICATOR);

    initUI();
    initConnect();
}

CloseAllDialogIndicator::~CloseAllDialogIndicator()
{

}

void CloseAllDialogIndicator::initUI()
{
    resize(QSize(400, 50));

    m_messageLabel = new QLabel(this);
    AC_SET_OBJECT_NAME(m_messageLabel, AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL);
    AC_SET_ACCESSIBLE_NAME(m_messageLabel, AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL);

    m_closeButton = new QPushButton(tr("Close all"), this);
    AC_SET_OBJECT_NAME(m_closeButton, AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON);
    AC_SET_ACCESSIBLE_NAME(m_closeButton, AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_messageLabel, Qt::AlignCenter);
    mainLayout->addSpacing(50);
    mainLayout->addWidget(m_closeButton, Qt::AlignRight);
    mainLayout->setContentsMargins(25, 5, 25, 5);
    setLayout(mainLayout);

    setTotalMessage(0, 0);
}

void CloseAllDialogIndicator::initConnect()
{
    connect(m_closeButton, &QPushButton::clicked, this, &CloseAllDialogIndicator::allClosed);
}

void CloseAllDialogIndicator::setTotalMessage(qint64 size, int count)
{
    QString message = tr("Total size: %1, %2 files").arg(FileUtils::formatSize(size), QString::number(count));
    m_messageLabel->setText(message);
}

void CloseAllDialogIndicator::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape){
        return;
    }
    QDialog::keyPressEvent(event);
}

void CloseAllDialogIndicator::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    //! task 36981 使用Dbus获取dock高度计算对话框显示位置
    QDBusInterface deepin_dockInfo("com.deepin.dde.daemon.Dock",
                                   "/com/deepin/dde/daemon/Dock",
                                   "com.deepin.dde.daemon.Dock",
                                   QDBusConnection::sessionBus(), this);

    int dockHeight = 0;
    if(deepin_dockInfo.isValid())
    {
        QVariant temp = deepin_dockInfo.property("WindowSizeEfficient");
        dockHeight = temp.toInt();
    }

    QRect screenGeometry = qApp->desktop()->availableGeometry();

    int geometryHeight = screenGeometry.height() - dockHeight;
    int geometryWidth = screenGeometry.width();

    move((geometryWidth - width()) / 2, geometryHeight - height());

    return DAbstractDialog::showEvent(event);
}

