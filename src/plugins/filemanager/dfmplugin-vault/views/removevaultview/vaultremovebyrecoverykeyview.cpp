// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovebyrecoverykeyview.h"

#include <DToolTip>
#include <DThemeManager>
#include <DFloatingWidget>

#include <QVBoxLayout>
#include <QTimer>
#include <QPlainTextEdit>

#define MAX_KEY_LENGTH (32)   //!凭证最大值，4的倍数

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultRemoveByRecoverykeyView::VaultRemoveByRecoverykeyView(QWidget *parent)
    : QWidget(parent)
{
    keyEdit = new QPlainTextEdit(this);
    keyEdit->setPlaceholderText(tr("Input the 32-digit recovery key"));
    keyEdit->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(keyEdit);
    layout->setMargin(0);
    this->setLayout(layout);

    connect(keyEdit, &QPlainTextEdit::textChanged, this, &VaultRemoveByRecoverykeyView::onRecoveryKeyChanged);
}

VaultRemoveByRecoverykeyView::~VaultRemoveByRecoverykeyView()
{
    if (tooltip) {
        tooltip->deleteLater();
    }
}

QString VaultRemoveByRecoverykeyView::getRecoverykey()
{
    QString strKey = keyEdit->toPlainText();

    return strKey.replace("-", "");
}

void VaultRemoveByRecoverykeyView::clear()
{
    keyEdit->clear();
}

void VaultRemoveByRecoverykeyView::showAlertMessage(const QString &text, int duration)
{

    if (!tooltip) {
        tooltip = new DToolTip(text);
        tooltip->setObjectName("AlertTooltip");
        tooltip->setForegroundRole(DPalette::TextWarning);
        tooltip->setWordWrap(true);

        floatWidget = new DFloatingWidget;
        floatWidget->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        floatWidget->setBackgroundRole(QPalette::ToolTipBase);
        floatWidget->setWidget(tooltip);
    }

    floatWidget->setParent(parentWidget());

    tooltip->setText(text);
    if (floatWidget->parent()) {
        floatWidget->setGeometry(0, 25, 68, 26);
        floatWidget->show();
        floatWidget->adjustSize();
        floatWidget->raise();
    }

    if (duration < 0) {
        return;
    }

    QTimer::singleShot(duration, floatWidget, [=] {
        floatWidget->close();
    });
}

void VaultRemoveByRecoverykeyView::onRecoveryKeyChanged()
{
    QString key = keyEdit->toPlainText();
    int length = key.length();
    int maxLength = MAX_KEY_LENGTH + 7;

    //! 限制密钥输入框只能输入数字、字母、以及+/-
    QRegExp rx("[a-zA-Z0-9-+/]+");
    QString res("");
    int pos = 0;
    while ((pos = rx.indexIn(key, pos)) != -1) {
        res += rx.cap(0);
        pos += rx.matchedLength();
    }
    key = res;

    keyEdit->blockSignals(true);
    //! 限制输入的最大长度
    if (length > maxLength) {
        int position = keyEdit->textCursor().position();
        QTextCursor textCursor = keyEdit->textCursor();
        key.remove(position - (length - maxLength), length - maxLength);
        keyEdit->setPlainText(key);
        textCursor.setPosition(position - (length - maxLength));
        keyEdit->setTextCursor(textCursor);

        keyEdit->blockSignals(false);
        return;
    }

    int position = afterRecoveryKeyChanged(key);
    keyEdit->setPlainText(key);

    QTextCursor textCursor = keyEdit->textCursor();
    textCursor.setPosition(position);
    keyEdit->setTextCursor(textCursor);
    keyEdit->blockSignals(false);
}

int VaultRemoveByRecoverykeyView::afterRecoveryKeyChanged(QString &str)
{
    if (str.isEmpty()) {
        return -1;
    }

    int location = keyEdit->textCursor().position();   //! 计算当前光标位置
    int srcLength = str.length();   //! 用于计算原有字符串中的“-”数量
    //!清除所有的“-”
    str.replace("-", "");
    int minusNumber = srcLength - str.length();   //! 原有字符串中的“-”数量

    int index = 4;
    int minusNum = 0;

    int length = str.length();
    while (index < length) {
        if (index % 4 == 0) {
            str.insert(index + minusNum, "-");
            minusNum++;
        }
        index++;
    }

    //!计算添加“-”后，重新计算下光标的位置，
    if (minusNum > minusNumber) {
        location += minusNum - minusNumber;
    }

    if (location > str.length()) {
        location = str.length();
    } else if (location < 0) {
        location = 0;
    }

    return location;
}

bool VaultRemoveByRecoverykeyView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(watched);
        if (edit == keyEdit) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

            //! 过滤换行操作以及“-”
            if (keyEvent->key() == Qt::Key_Enter
                || keyEvent->key() == Qt::Key_Return
                || keyEvent->key() == Qt::Key_Minus) {
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}
