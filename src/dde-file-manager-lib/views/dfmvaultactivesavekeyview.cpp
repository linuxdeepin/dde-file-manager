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

#include "dfmvaultactivesavekeyview.h"
#include "operatorcenter.h"
#include "accessibility/ac-lib-file-manager.h"

#include <DButtonBox>
#include <DLabel>

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QPlainTextEdit>

DFMVaultActiveSaveKeyView::DFMVaultActiveSaveKeyView(QWidget *parent)
    : QWidget(parent)
    , m_pTipsLabel(nullptr)
    , m_pQRCodeImage(nullptr)
    , m_pNext(nullptr)
{
    AC_SET_ACCESSIBLE_NAME(this, AC_VAULT_ACTIVE_KEY_WIDGET);

    // 标题名
    QLabel *pLabelTitle = new QLabel(tr("Recovery Key"), this);
    AC_SET_ACCESSIBLE_NAME(pLabelTitle, AC_VAULT_ACTIVE_KEY_TITLE);
    QFont font = pLabelTitle->font();
    font.setPixelSize(18);
    pLabelTitle->setFont(font);
    pLabelTitle->setAlignment(Qt::AlignHCenter);

    // 提示标签
    m_pTipsLabel = new DLabel(tr("Generate a recovery key in case that you forgot the password"), this);
    AC_SET_ACCESSIBLE_NAME(m_pTipsLabel, AC_VAULT_ACTIVE_KEY_CONTENT);

    DButtonBox *m_pButtonBox = new DButtonBox(this);

    // 密钥
    m_pKeyBtn = new DButtonBoxButton(tr("Key"), this);
    AC_SET_ACCESSIBLE_NAME(m_pKeyBtn, AC_VAULT_ACTIVE_KEY_KEY_BUTTON);
    m_pKeyBtn->setCheckable(true);
    m_pKeyBtn->setChecked(true);
    connect(m_pKeyBtn, &DButtonBoxButton::clicked,
            this, &DFMVaultActiveSaveKeyView::slotKeyBtnClicked);
    m_pKeyText = new QPlainTextEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_pKeyText, AC_VAULT_ACTIVE_KEY_KEY_EDIT);
    m_pKeyText->setReadOnly(true);
    m_pKeyText->setFixedSize(452, 134);

    // 二维码
    m_pQRCodeBtn = new DButtonBoxButton(tr("QR code"), this);
    AC_SET_ACCESSIBLE_NAME(m_pQRCodeBtn, AC_VAULT_ACTIVE_KEY_QRCODE_BUTTON);
    m_pQRCodeBtn->setCheckable(true);
    connect(m_pQRCodeBtn, &DButtonBoxButton::clicked,
            this, &DFMVaultActiveSaveKeyView::slotQRCodeBtnClicked);
    m_pQRCodeImage = new DLabel(this);
    AC_SET_ACCESSIBLE_NAME(m_pQRCodeImage, AC_VAULT_ACTIVE_KEY_QRCODE_IMAGE);
    m_pQRCodeImage->setFixedSize(120, 120);
    m_pQRCodeImage->setVisible(false);

    QList<DButtonBoxButton *> lstBtn;
    lstBtn.push_back(m_pKeyBtn);
    lstBtn.push_back(m_pQRCodeBtn);
    m_pButtonBox->setButtonList(lstBtn, true);

    // 扫描提示
    m_pScanTipsLabel = new DLabel(tr("Scan QR code and save the key to another device"), this);
    AC_SET_ACCESSIBLE_NAME(m_pScanTipsLabel, AC_VAULT_ACTIVE_KEY_QRCODE_HINT);
    QFont font2 = m_pScanTipsLabel->font();
    font2.setPixelSize(12);
    m_pScanTipsLabel->setFont(font2);
    m_pScanTipsLabel->setVisible(false);

    // 下一步按钮
    m_pNext = new QPushButton(tr("Next"), this);
    AC_SET_ACCESSIBLE_NAME(m_pNext, AC_VAULT_ACTIVE_KEY_NEXT_BUTTON);
    connect(m_pNext, &QPushButton::clicked,
            this, &DFMVaultActiveSaveKeyView::slotNextBtnClicked);

    // 布局
    play1 = new QGridLayout();
    play1->setMargin(0);
    play1->addWidget(m_pTipsLabel, 0, 0, 1, 4, Qt::AlignCenter);
    play1->addWidget(m_pButtonBox, 1, 0, 1, 4, Qt::AlignHCenter);
    play1->addWidget(m_pKeyText, 2, 0, 2, 4, Qt::AlignCenter);


    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabelTitle);
    play->addLayout(play1);
    play->addStretch();
    play->addWidget(m_pNext);
}

void DFMVaultActiveSaveKeyView::showEvent(QShowEvent *event)
{
    QString strUserKey = OperatorCenter::getInstance()->getUserKey();
    qDebug() << "user key: " << strUserKey;

    QString strKeyShow = strUserKey;
    strKeyShow.insert(28, '-');
    strKeyShow.insert(24, '-');
    strKeyShow.insert(20, '-');
    strKeyShow.insert(16, '-');
    strKeyShow.insert(12, '-');
    strKeyShow.insert(8, '-');
    strKeyShow.insert(4, '-');
    QString strContent = QString(tr("Recovery Key:")) + strKeyShow;
    QPixmap QRCodePix;
    if (OperatorCenter::getInstance()->createQRCode(strContent, m_pQRCodeImage->width(), m_pQRCodeImage->height(), QRCodePix))
        m_pQRCodeImage->setPixmap(QRCodePix);
    m_pKeyText->setPlainText(strKeyShow);

    QWidget::showEvent(event);
}

void DFMVaultActiveSaveKeyView::slotNextBtnClicked()
{
    emit sigAccepted();
}

void DFMVaultActiveSaveKeyView::slotKeyBtnClicked()
{
    play1->removeWidget(m_pQRCodeImage);
    m_pQRCodeImage->setVisible(false);
    play1->removeWidget(m_pScanTipsLabel);
    m_pScanTipsLabel->setVisible(false);

    play1->addWidget(m_pKeyText, 2, 0, 2, 4, Qt::AlignCenter);
    m_pKeyText->setVisible(true);
}

void DFMVaultActiveSaveKeyView::slotQRCodeBtnClicked()
{
    m_pKeyText->setVisible(false);
    play1->removeWidget(m_pKeyText);

    play1->addWidget(m_pQRCodeImage, 2, 0, 1, 4, Qt::AlignHCenter);
    m_pQRCodeImage->setVisible(true);
    play1->addWidget(m_pScanTipsLabel, 3, 0, 1, 4, Qt::AlignHCenter);
    m_pScanTipsLabel->setVisible(true);
}
