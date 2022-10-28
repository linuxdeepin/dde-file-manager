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

CloseAllDialogIndicator::CloseAllDialogIndicator(QWidget *parent)
    : DAbstractDialog(parent)
{
    setWindowFlags(windowFlags() /*| Qt::FramelessWindowHint*/ | Qt::WindowStaysOnTopHint);
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
    m_messageLabel = new QLabel(this);
    auto font = m_messageLabel->font();
    font.setPixelSize(12);
    m_messageLabel->setFont(font);
    //    AC_SET_OBJECT_NAME(m_messageLabel, AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL);
    //    AC_SET_ACCESSIBLE_NAME(m_messageLabel, AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL);

    m_closeButton = new DCommandLinkButton(tr("Close all"), this);
    font = m_closeButton->font();
    font.setPixelSize(14);
    m_closeButton->setFont(font);
    //    AC_SET_OBJECT_NAME(m_closeButton, AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON);
    //    AC_SET_ACCESSIBLE_NAME(m_closeButton, AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_messageLabel, Qt::AlignCenter);
    mainLayout->addWidget(m_closeButton, Qt::AlignRight);
    mainLayout->addSpacing(0);
    mainLayout->setContentsMargins(15, 8, 12, 11);
    setLayout(mainLayout);

    setTotalMessage(0, 0);

    setFixedSize(297, 36);

    //    messageLabel->setStyleSheet("border: 1px solid black;");
    //    closeButton->setStyleSheet("border: 1px solid red;");
}

void CloseAllDialogIndicator::initConnect()
{
    connect(m_closeButton, &QPushButton::clicked, this, &CloseAllDialogIndicator::allClosed);
}

void CloseAllDialogIndicator::setTotalMessage(qint64 size, int count)
{
    QString message = tr("Total size: %1, %2 files").arg(FileUtils::formatSize(size), QString::number(count));
    QFontMetrics fm(m_messageLabel->fontMetrics());
    int txtWidth = fm.width(message);
    int charWidth = fm.width(message[0]);
    m_messageLabel->setText(message);
    if (txtWidth > m_messageLabel->width() - charWidth)   // consider the inner spacing of text label
        setFixedWidth(this->width() + charWidth);
}

void CloseAllDialogIndicator::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        return;
    }
    DAbstractDialog::keyPressEvent(event);
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
    if (deepin_dockInfo.isValid()) {
        QVariant temp = deepin_dockInfo.property("WindowSizeEfficient");
        dockHeight = temp.toInt();
    }

    QRect screenGeometry = qApp->desktop()->availableGeometry();

    int geometryHeight = screenGeometry.height() - dockHeight;
    int geometryWidth = screenGeometry.width();

    move((geometryWidth - width()) / 2, geometryHeight - height());

    return DAbstractDialog::showEvent(event);
}
