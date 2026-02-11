// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "closealldialog.h"
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/windowutils.h>

#include <DLabel>
#include <DCommandLinkButton>

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QFontMetrics>
#include <QScreen>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

constexpr int kFontSize = 13;

CloseAllDialog::CloseAllDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    setWindowFlags(windowFlags() /*| Qt::FramelessWindowHint*/ | Qt::WindowStaysOnTopHint);
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
    messageLabel = new DLabel(this);
    auto font = messageLabel->font();
    font.setPixelSize(kFontSize);
    messageLabel->setFont(font);
    //    AC_SET_OBJECT_NAME(messageLabel, AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL);
    //    AC_SET_ACCESSIBLE_NAME(messageLabel, AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL);

    closeButton = new DCommandLinkButton(tr("Close all"), this);
    // 设置按钮无焦点
    closeButton->setFocusPolicy(Qt::NoFocus);
    font = closeButton->font();
    closeButton->setFont(font);
    //    AC_SET_OBJECT_NAME(closeButton, AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON);
    //    AC_SET_ACCESSIBLE_NAME(closeButton, AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(messageLabel, Qt::AlignCenter);
    mainLayout->addWidget(closeButton, Qt::AlignRight);
    mainLayout->addSpacing(0);
    mainLayout->setContentsMargins(15, 9, 12, 10);
    setLayout(mainLayout);

    setTotalMessage(0, 0);

    setFixedSize(297, 36);

    //    messageLabel->setStyleSheet("border: 1px solid black;");
    //    closeButton->setStyleSheet("border: 1px solid red;");
}

void CloseAllDialog::initConnect()
{
    connect(closeButton, &QPushButton::clicked, this, &CloseAllDialog::allClosed);
}

void CloseAllDialog::setTotalMessage(qint64 size, int count)
{
    QString message = tr("Total size: %1, %2 files").arg(FileUtils::formatSize(size), QString::number(count));
    QFontMetrics fm(messageLabel->fontMetrics());
    int txtWidth = fm.horizontalAdvance(message);
    int charWidth = fm.horizontalAdvance(message[0]);
    messageLabel->setText(message);
    if (txtWidth > messageLabel->width() - charWidth)   // consider the inner spacing of text label
        setFixedWidth(this->width() + charWidth);
}

void CloseAllDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        return;
    }
    DAbstractDialog::keyPressEvent(event);
}

void CloseAllDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    QScreen *cursor_screen = WindowUtils::cursorScreen();
    QRect screenGeometry = cursor_screen->availableGeometry();
    int geometryHeight = screenGeometry.height() - UniversalUtils::dockHeight();
    int geometryWidth = screenGeometry.width();

    move(QPoint((geometryWidth - width()) / 2, geometryHeight - height()) + cursor_screen->geometry().topLeft());

    setTotalMessage(0, 0);
    return DAbstractDialog::showEvent(event);
}
