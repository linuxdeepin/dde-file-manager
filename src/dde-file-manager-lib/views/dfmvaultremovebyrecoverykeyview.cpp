/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "dfmvaultremovebyrecoverykeyview.h"
#include "accessibility/ac-lib-file-manager.h"

#include <dtkwidget_global.h>
#include <DToolTip>
#include <DThemeManager>
#include <DFloatingWidget>

#include <QVBoxLayout>
#include <QTimer>
#include <QPlainTextEdit>


#define MAX_KEY_LENGTH (32) //凭证最大值，4的倍数
DWIDGET_USE_NAMESPACE
class DFMVaultRemoveByRecoverykeyViewPrivate
{
public:
    explicit DFMVaultRemoveByRecoverykeyViewPrivate(DFMVaultRemoveByRecoverykeyView *qq)
        : q_ptr(qq) {}
    ~DFMVaultRemoveByRecoverykeyViewPrivate()
    {
        if (tooltip) {
            tooltip->deleteLater();
        }
    }

    DToolTip *tooltip {nullptr};
    DFloatingWidget *frame {nullptr};

    DFMVaultRemoveByRecoverykeyView *q_ptr;
    Q_DECLARE_PUBLIC(DFMVaultRemoveByRecoverykeyView)
};

DFMVaultRemoveByRecoverykeyView::DFMVaultRemoveByRecoverykeyView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new DFMVaultRemoveByRecoverykeyViewPrivate(this))
{
    m_keyEdit = new QPlainTextEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_keyEdit, AC_VAULT_DELETE_KEY_EDIT);
    m_keyEdit->setPlaceholderText(tr("Input the 32-digit recovery key"));
    m_keyEdit->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_keyEdit);
    layout->setMargin(0);
    this->setLayout(layout);

    connect(m_keyEdit, &QPlainTextEdit::textChanged, this, &DFMVaultRemoveByRecoverykeyView::onRecoveryKeyChanged);
}

DFMVaultRemoveByRecoverykeyView::~DFMVaultRemoveByRecoverykeyView()
{

}

QString DFMVaultRemoveByRecoverykeyView::getRecoverykey()
{
    QString strKey = m_keyEdit->toPlainText();

    return strKey.replace("-", "");
}

void DFMVaultRemoveByRecoverykeyView::clear()
{
    m_keyEdit->clear();
}

void DFMVaultRemoveByRecoverykeyView::showAlertMessage(const QString &text, int duration)
{
    Q_D(DFMVaultRemoveByRecoverykeyView);

    if (!d->tooltip) {
        d->tooltip = new DToolTip(text);
        d->tooltip->setObjectName("AlertTooltip");
        d->tooltip->setForegroundRole(DPalette::TextWarning);
        d->tooltip->setWordWrap(true);

        d->frame = new DFloatingWidget;
        d->frame->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        d->frame->setBackgroundRole(QPalette::ToolTipBase);
        d->frame->setWidget(d->tooltip);
    }

    d->frame->setParent(parentWidget());

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

void DFMVaultRemoveByRecoverykeyView::onRecoveryKeyChanged()
{
    QString key = m_keyEdit->toPlainText();
    int length = key.length();
    int maxLength = MAX_KEY_LENGTH + 7;

    // 限制密钥输入框只能输入数字、字母、以及+/-
    QRegExp rx("[a-zA-Z0-9-+/]+");
    QString res("");
    int pos = 0;
    while ((pos = rx.indexIn(key, pos)) != -1) {
        res += rx.cap(0);
        pos += rx.matchedLength();
    }
    key = res;

    m_keyEdit->blockSignals(true);
    // 限制输入的最大长度
    if (length > maxLength) {
        int position = m_keyEdit->textCursor().position();
        QTextCursor textCursor = m_keyEdit->textCursor();
        key.remove(position - (length - maxLength), length - maxLength);
        m_keyEdit->setPlainText(key);
        textCursor.setPosition(position - (length - maxLength));
        m_keyEdit->setTextCursor(textCursor);

        m_keyEdit->blockSignals(false);
        return;
    }

    int position = afterRecoveryKeyChanged(key);
    m_keyEdit->setPlainText(key);

    QTextCursor textCursor = m_keyEdit->textCursor();
    textCursor.setPosition(position);
    m_keyEdit->setTextCursor(textCursor);
    m_keyEdit->blockSignals(false);
}

int DFMVaultRemoveByRecoverykeyView::afterRecoveryKeyChanged(QString &str)
{
    if (str.isEmpty()) {
        return -1;
    }

    int location = m_keyEdit->textCursor().position(); // 计算当前光标位置
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

bool DFMVaultRemoveByRecoverykeyView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(watched);
        if (edit == m_keyEdit) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

            // 过滤换行操作以及“-”
            if (keyEvent->key() == Qt::Key_Enter
                    || keyEvent->key() == Qt::Key_Return
                    || keyEvent->key() == Qt::Key_Minus) {
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}
