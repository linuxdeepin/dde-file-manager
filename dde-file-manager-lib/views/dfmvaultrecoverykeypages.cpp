/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmvaultrecoverykeypages.h"
#include "vault/interfaceactivevault.h"
#include "controllers/vaultcontroller.h"

#include <QPlainTextEdit>
#include <QAbstractButton>

// 密钥最大长度
#define MAX_KEY_LENGTH (32)

DFMVaultRecoveryKeyPages::DFMVaultRecoveryKeyPages(QWidget *parent)
    : DDialog (parent)
{
    this->setTitle(tr("Unlock by Key"));
    this->setIcon(QIcon::fromTheme("dfm_safebox"));
    this->setFixedSize(438, 260);

    QStringList btnList({tr("Cancel"), tr("Unlock")});
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);
    getButton(1)->setEnabled(false);

    m_recoveryKeyEdit = new QPlainTextEdit(this);
    m_recoveryKeyEdit->setPlaceholderText(tr("Input the 32-digit recovery key"));
    m_recoveryKeyEdit->setMaximumBlockCount(MAX_KEY_LENGTH + 3);
    m_recoveryKeyEdit->installEventFilter(this);

    addContent(m_recoveryKeyEdit);
    // 防止点击按钮后界面隐藏
    setOnButtonClickedClose(false);

    connect(this, &DFMVaultRecoveryKeyPages::buttonClicked, this, &DFMVaultRecoveryKeyPages::onButtonClicked);
    connect(m_recoveryKeyEdit, &QPlainTextEdit::textChanged, this, &DFMVaultRecoveryKeyPages::recoveryKeyChanged);
    connect(VaultController::getVaultController(), &VaultController::signalUnlockVault, this, &DFMVaultRecoveryKeyPages::onUnlockVault);
}

DFMVaultRecoveryKeyPages *DFMVaultRecoveryKeyPages::instance()
{
    static DFMVaultRecoveryKeyPages s_instance;
    return &s_instance;
}

void DFMVaultRecoveryKeyPages::onButtonClicked(const int &index)
{
    if (index == 1){
        QString strKey = m_recoveryKeyEdit->toPlainText();
        strKey.replace(tr("-"), tr(""));

        QString strClipher("");
        if (InterfaceActiveVault::checkUserKey(strKey, strClipher)){
            VaultController::getVaultController()->unlockVault(strClipher);
        }

        return;
    }

    // 重置所有控件状态
    {
        m_recoveryKeyEdit->clear();
    }
    close();
}

int DFMVaultRecoveryKeyPages::afterRecoveryKeyChanged(QString &str)
{
    if (str.isEmpty()){
        return -1;
    }

    int location = m_recoveryKeyEdit->textCursor().position(); // 计算当前光标位置
    int srcLength = str.length();   // 用于计算原有字符串中的“-”数量
    //清除所有的“-”
    str.replace(tr("-"), tr(""));
    int minusNumber = srcLength - str.length(); // 原有字符串中的“-”数量

    int index = 4;
    int minusNum = 0;

    int length = str.length();
    while (index < length) {
        if (index % 4 == 0){
            str.insert(index + minusNum, tr("-"));
            minusNum++;
        }
        index++;
    }

    //计算添加“-”后，重新计算下光标的位置，
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

void DFMVaultRecoveryKeyPages::recoveryKeyChanged()
{
    QString key = m_recoveryKeyEdit->toPlainText();
    int length = key.length();
    int maxLength = MAX_KEY_LENGTH + 7;

    if (key.isEmpty()){
        getButton(1)->setEnabled(false);
    } else {
        getButton(1)->setEnabled(true);
    }

    // 限制输入的最大长度
    if (length > maxLength){
        int position = m_recoveryKeyEdit->textCursor().position();
        QTextCursor textCursor = m_recoveryKeyEdit->textCursor();
        key.remove(position-(length - maxLength), length - maxLength);
        m_recoveryKeyEdit->setPlainText(key);
        textCursor.setPosition(position - (length-maxLength));
        m_recoveryKeyEdit->setTextCursor(textCursor);

        return;
    }

    static bool isEdited = false;
    if (!isEdited){
        isEdited = true;
        int position = afterRecoveryKeyChanged(key);
        m_recoveryKeyEdit->setPlainText(key);

        QTextCursor textCursor = m_recoveryKeyEdit->textCursor();
        textCursor.setPosition(position);
        m_recoveryKeyEdit->setTextCursor(textCursor);
    } else {
        isEdited = false;
    }
}

void DFMVaultRecoveryKeyPages::onUnlockVault(int state)
{
    if (state == 0){
        m_recoveryKeyEdit->clear();
        accept();
    }
}

bool DFMVaultRecoveryKeyPages::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress){
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(watched);
        if (edit == m_recoveryKeyEdit) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

            switch (keyEvent->key()) {
            case Qt::Key_Minus:
            case Qt::Key_Enter:
            case Qt::Key_Return:    // 过滤"-"以及换行操作
                return true;
            default:
                break;
            }
        }
    }

    return DDialog::eventFilter(watched, event);
}

