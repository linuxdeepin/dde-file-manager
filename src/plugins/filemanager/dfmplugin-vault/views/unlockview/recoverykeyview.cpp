// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recoverykeyview.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/servicemanager.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/fileencrypthandle.h"
#include "utils/vaultautolock.h"

#include <DToolTip>
#include <DFloatingWidget>
#include <DDialog>

#include <QPlainTextEdit>
#include <QAbstractButton>
#include <QTimer>
#include <QVBoxLayout>

//! 密钥最大长度
#define MAX_KEY_LENGTH (32)

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

RecoveryKeyView::RecoveryKeyView(QWidget *parent)
    : QFrame(parent)
{
    //! 密钥编辑框
    recoveryKeyEdit = new QPlainTextEdit(this);
    recoveryKeyEdit->setPlaceholderText(tr("Input the 32-digit recovery key"));
    recoveryKeyEdit->setMaximumBlockCount(MAX_KEY_LENGTH + 3);
    recoveryKeyEdit->installEventFilter(this);

    //! 布局
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(recoveryKeyEdit);
    setLayout(mainLayout);

    connect(recoveryKeyEdit, &QPlainTextEdit::textChanged, this, &RecoveryKeyView::recoveryKeyChanged);
}

RecoveryKeyView::~RecoveryKeyView()
{
    if (tooltip) {
        tooltip->deleteLater();
    }
}

QStringList RecoveryKeyView::btnText()
{
    return { tr("Cancel", "button"), tr("Unlock", "button") };
}

QString RecoveryKeyView::titleText()
{
    return QString(tr("Unlock by Key"));
}

void RecoveryKeyView::showAlertMessage(const QString &text, int duration)
{
    if (!tooltip) {
        tooltip = new DToolTip(text);
        tooltip->setObjectName("AlertTooltip");
        tooltip->setForegroundRole(DPalette::TextWarning);
        tooltip->setWordWrap(true);

        floatWidget = new DFloatingWidget;
        floatWidget->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        floatWidget->setStyleSheet("background-color: rgba(247, 247, 247, 0.6);");
        floatWidget->setWidget(tooltip);
    }

    floatWidget->setParent(recoveryKeyEdit);

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

void RecoveryKeyView::buttonClicked(int index, const QString &text)
{
    if (index == 1) {   // unlock vault
        //! 点击解锁后，灰化解锁按钮
        emit sigBtnEnabled(1, false);

        QString strKey = recoveryKeyEdit->toPlainText();
        strKey.replace("-", "");

        QString strCipher("");
        if (InterfaceActiveVault::checkUserKey(strKey, strCipher)) {
            unlockByKey = true;
            QString encryptBaseDir = PathManager::vaultLockPath();
            QString decryptFileDir = PathManager::vaultUnlockPath();
            bool result = FileEncryptHandle::instance()->unlockVault(encryptBaseDir, decryptFileDir, strCipher);
            handleUnlockVault(result);
        } else {
            showAlertMessage(tr("Wrong recovery key"));
        }
        emit sigBtnEnabled(1, true);
        return;
    } else {
        emit sigCloseDialog();
    }
}

int RecoveryKeyView::afterRecoveryKeyChanged(QString &str)
{
    if (str.isEmpty()) {
        return -1;
    }

    int location = recoveryKeyEdit->textCursor().position();   //! 计算当前光标位置
    int srcLength = str.length();   //! 用于计算原有字符串中的“-”数量
    //! 清除所有的“-”
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

    //! 计算添加“-”后，重新计算下光标的位置，
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

void RecoveryKeyView::showEvent(QShowEvent *event)
{
    recoveryKeyEdit->clear();
    unlockByKey = false;
    event->accept();
}

void RecoveryKeyView::recoveryKeyChanged()
{
    QString key = recoveryKeyEdit->toPlainText();
    int length = key.length();
    int maxLength = MAX_KEY_LENGTH + 7;

    if (key.isEmpty()) {
        emit sigBtnEnabled(1, false);
    } else {
        emit sigBtnEnabled(1, true);
    }

    //! 限制密钥输入框只能输入数字、字母、以及+/-
    QRegularExpression rx("[a-zA-Z0-9-+/]+");
    QString res;
    QRegularExpressionMatch match;
    int pos = 0;

    while ((match = rx.match(key, pos)).hasMatch()) {
        res += match.captured(0);
        pos = match.capturedEnd();
    }

    key = res;

    recoveryKeyEdit->blockSignals(true);
    //! 限制输入的最大长度
    if (length > maxLength) {
        int position = recoveryKeyEdit->textCursor().position();
        QTextCursor textCursor = recoveryKeyEdit->textCursor();
        key.remove(position - (length - maxLength), length - maxLength);
        recoveryKeyEdit->setPlainText(key);
        textCursor.setPosition(position - (length - maxLength));
        recoveryKeyEdit->setTextCursor(textCursor);

        recoveryKeyEdit->blockSignals(false);
        return;
    }

    int position = afterRecoveryKeyChanged(key);
    recoveryKeyEdit->setPlainText(key);

    QTextCursor textCursor = recoveryKeyEdit->textCursor();
    textCursor.setPosition(position);
    recoveryKeyEdit->setTextCursor(textCursor);

    recoveryKeyEdit->blockSignals(false);
}

void RecoveryKeyView::handleUnlockVault(bool result)
{
    if (unlockByKey) {
        if (result) {
            //! success
            VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(),
                                                     VaultHelper::instance()->rootUrl());
            VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
            VaultAutoLock::instance()->slotUnlockVault(0);
            emit sigCloseDialog();
        } else {
            //! others
            QString errMsg = tr("Failed to unlock file vault");
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.setTitle(errMsg);
            dialog.addButton(tr("OK"), true, DDialog::ButtonRecommend);
            dialog.exec();
        }
        unlockByKey = false;
    }
}

bool RecoveryKeyView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(watched);
        if (edit == recoveryKeyEdit) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

            switch (keyEvent->key()) {
            case Qt::Key_Minus:
            case Qt::Key_Enter:
            case Qt::Key_Return:   //! 过滤"-"以及换行操作
                return true;
            default:
                break;
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}
