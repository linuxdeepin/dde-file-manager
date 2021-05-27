/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "accessibility/ac-lib-file-manager.h"

#include <DToolTip>
#include <DFloatingWidget>

#include <QPlainTextEdit>
#include <QAbstractButton>
#include <QTimer>
#include <QVBoxLayout>

// 密钥最大长度
#define MAX_KEY_LENGTH (32)
DWIDGET_USE_NAMESPACE
class DFMVaultRecoveryKeyPagesPrivate
{
public:
    ~DFMVaultRecoveryKeyPagesPrivate()
    {
        if (tooltip) {
            tooltip->deleteLater();
        }
    }

    DToolTip *tooltip {nullptr};
    DFloatingWidget *frame {nullptr};
};

DFMVaultRecoveryKeyPages::DFMVaultRecoveryKeyPages(QWidget *parent)
    : DFMVaultPageBase(parent)
    , d_ptr(new DFMVaultRecoveryKeyPagesPrivate())
{
    AC_SET_ACCESSIBLE_NAME(this, AC_VAULT_KEY_UNLOCK_WIDGET);

    this->setIcon(QIcon::fromTheme("dfm_vault"));
    this->setFixedSize(396, 218);

    // 标题
    QLabel *pTitle = new QLabel(tr("Unlock by Key"), this);
    AC_SET_ACCESSIBLE_NAME(pTitle, AC_VAULT_KEY_UNLOCK_TITLE);
    QFont font = pTitle->font();
    font.setPixelSize(16);
    pTitle->setFont(font);
    pTitle->setAlignment(Qt::AlignHCenter);

    // 密钥编辑框
    m_recoveryKeyEdit = new QPlainTextEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_recoveryKeyEdit, AC_VAULT_KEY_UNLOCK_EDIT);
    m_recoveryKeyEdit->setPlaceholderText(tr("Input the 32-digit recovery key"));
    m_recoveryKeyEdit->setMaximumBlockCount(MAX_KEY_LENGTH + 3);
    m_recoveryKeyEdit->installEventFilter(this);

    // 主视图
    QFrame *mainFrame = new QFrame(this);
    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setMargin(0);
    mainLayout->addWidget(pTitle);
    mainLayout->addWidget(m_recoveryKeyEdit);

    mainFrame->setLayout(mainLayout);
    addContent(mainFrame);

    QStringList btnList({tr("Cancel","button"), tr("Unlock","button")});
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);
    getButton(1)->setEnabled(false);

    AC_SET_ACCESSIBLE_NAME(getButton(0), AC_VAULT_KEY_UNLOCK_CANCEL_BUTTON);
    AC_SET_ACCESSIBLE_NAME(getButton(1), AC_VAULT_KEY_UNLOCK_OK_BUTTON);

    // 防止点击按钮后界面隐藏
    setOnButtonClickedClose(false);

    connect(this, &DFMVaultRecoveryKeyPages::buttonClicked, this, &DFMVaultRecoveryKeyPages::onButtonClicked);
    connect(m_recoveryKeyEdit, &QPlainTextEdit::textChanged, this, &DFMVaultRecoveryKeyPages::recoveryKeyChanged);
    connect(VaultController::ins(), &VaultController::signalUnlockVault, this, &DFMVaultRecoveryKeyPages::onUnlockVault);
    connect(this, &DFMVaultPageBase::accepted, this, &DFMVaultPageBase::enterVaultDir);
}

DFMVaultRecoveryKeyPages::~DFMVaultRecoveryKeyPages()
{

}

DFMVaultRecoveryKeyPages *DFMVaultRecoveryKeyPages::instance()
{
    static DFMVaultRecoveryKeyPages s_instance;
    return &s_instance;
}

void DFMVaultRecoveryKeyPages::showAlertMessage(const QString &text, int duration)
{
    Q_D(DFMVaultRecoveryKeyPages);

    if (!d->tooltip) {
        d->tooltip = new DToolTip(text);
        d->tooltip->setObjectName("AlertTooltip");
        d->tooltip->setForegroundRole(DPalette::TextWarning);
        d->tooltip->setWordWrap(true);

        d->frame = new DFloatingWidget;
        d->frame->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        d->frame->setStyleSheet("background-color: rgba(247, 247, 247, 0.6);");
        d->frame->setWidget(d->tooltip);
    }

    d->frame->setParent(m_recoveryKeyEdit);

    d->tooltip->setText(text);
    if (d->frame->parent()) {
        d->frame->setGeometry(0, 25, 68, 26);
        d->frame->show();
        d->frame->adjustSize();
        d->frame->raise();
    }

    if (duration < 0) {
        return;
    }

    QTimer::singleShot(duration, d->frame, [d] {
        d->frame->close();
    });
}

void DFMVaultRecoveryKeyPages::onButtonClicked(const int &index)
{
    if (index == 1) {
        // 点击解锁后，灰化解锁按钮
        getButton(1)->setEnabled(false);

        QString strKey = m_recoveryKeyEdit->toPlainText();
        strKey.replace("-", "");

        QString strCipher("");
        if (InterfaceActiveVault::checkUserKey(strKey, strCipher)) {
            m_bUnlockByKey = true;
            VaultController::ins()->unlockVault(strCipher);
        } else {
            showAlertMessage(tr("Wrong recovery key"));
        }

        return;
    }

    close();
}

int DFMVaultRecoveryKeyPages::afterRecoveryKeyChanged(QString &str)
{
    if (str.isEmpty()) {
        return -1;
    }

    int location = m_recoveryKeyEdit->textCursor().position(); // 计算当前光标位置
    int srcLength = str.length();   // 用于计算原有字符串中的“-”数量
    //清除所有的“-”
    str.replace("-", "");
    int minusNumber = srcLength - str.length(); // 原有字符串中的“-”数量

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

void DFMVaultRecoveryKeyPages::showEvent(QShowEvent *event)
{
    m_recoveryKeyEdit->clear();
    m_bUnlockByKey = false;
    event->accept();
}

void DFMVaultRecoveryKeyPages::recoveryKeyChanged()
{
    QString key = m_recoveryKeyEdit->toPlainText();
    int length = key.length();
    int maxLength = MAX_KEY_LENGTH + 7;

    if (key.isEmpty()) {
        getButton(1)->setEnabled(false);
    } else {
        getButton(1)->setEnabled(true);
    }

    // 限制密钥输入框只能输入数字、字母、以及+/-
    QRegExp rx("[a-zA-Z0-9-+/]+");
    QString res("");
    int pos = 0;
    while ((pos = rx.indexIn(key, pos)) != -1) {
        res += rx.cap(0);
        pos += rx.matchedLength();
    }
    key = res;

    m_recoveryKeyEdit->blockSignals(true);
    // 限制输入的最大长度
    if (length > maxLength) {
        int position = m_recoveryKeyEdit->textCursor().position();
        QTextCursor textCursor = m_recoveryKeyEdit->textCursor();
        key.remove(position - (length - maxLength), length - maxLength);
        m_recoveryKeyEdit->setPlainText(key);
        textCursor.setPosition(position - (length - maxLength));
        m_recoveryKeyEdit->setTextCursor(textCursor);

        m_recoveryKeyEdit->blockSignals(false);
        return;
    }

    int position = afterRecoveryKeyChanged(key);
    m_recoveryKeyEdit->setPlainText(key);

    QTextCursor textCursor = m_recoveryKeyEdit->textCursor();
    textCursor.setPosition(position);
    m_recoveryKeyEdit->setTextCursor(textCursor);

    m_recoveryKeyEdit->blockSignals(false);
}

void DFMVaultRecoveryKeyPages::onUnlockVault(int state)
{
    if (m_bUnlockByKey) {
        if (state == 0) {
            // success
            emit accepted();
            close();
        } else {
            // others
            QString errMsg = tr("Failed to unlock file vault");
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.setTitle(errMsg);
            dialog.addButton(tr("OK"), true, DDialog::ButtonRecommend);
            dialog.exec();
        }
        m_bUnlockByKey = false;
    }
}

bool DFMVaultRecoveryKeyPages::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
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

