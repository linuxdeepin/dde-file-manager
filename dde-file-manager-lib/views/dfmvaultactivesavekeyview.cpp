#include "dfmvaultactivesavekeyview.h"
#include "operatorcenter.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include <QHBoxLayout>
#include <DButtonBox>
#include <DLabel>
#include <QPlainTextEdit>

DFMVaultActiveSaveKeyView::DFMVaultActiveSaveKeyView(QWidget *parent)
    : QWidget(parent)
    , m_pTipsLabel(nullptr)
    , m_pQRCodeImage(nullptr)
    , m_pNext(nullptr)
{
    // 标题名
    QLabel *pLabelTitle = new QLabel(tr("Recovery Key"), this);
    QFont font = pLabelTitle->font();
    font.setPixelSize(18);
    pLabelTitle->setFont(font);
    pLabelTitle->setAlignment(Qt::AlignHCenter);

    // 提示标签
    m_pTipsLabel = new DLabel(tr("Generate a recovery key in case that you forget the password"), this);

    DButtonBox *m_pButtonBox = new DButtonBox(this);
    m_pButtonBox->setFixedSize(200, 36);

    // 密钥
    m_pKeyBtn = new DButtonBoxButton(tr("Key"), this);
    m_pKeyBtn->setCheckable(true);
    m_pKeyBtn->setChecked(true);
    connect(m_pKeyBtn, &DButtonBoxButton::clicked,
            this, &DFMVaultActiveSaveKeyView::slotKeyBtnClicked);
    m_pKeyText = new QPlainTextEdit(this);
    m_pKeyText->setReadOnly(true);
    m_pKeyText->setFixedSize(452, 134);

    // 二维码
    m_pQRCodeBtn = new DButtonBoxButton(tr("QR code"), this);
    m_pQRCodeBtn->setCheckable(true);
    connect(m_pQRCodeBtn, &DButtonBoxButton::clicked,
            this, &DFMVaultActiveSaveKeyView::slotQRCodeBtnClicked);
    m_pQRCodeImage = new DLabel(this);
    m_pQRCodeImage->setFixedSize(120, 120);
    m_pQRCodeImage->setVisible(false);

    QList<DButtonBoxButton*> lstBtn;
    lstBtn.push_back(m_pKeyBtn);
    lstBtn.push_back(m_pQRCodeBtn);
    m_pButtonBox->setButtonList(lstBtn, true);

    // 扫描提示
    m_pScanTipsLabel = new DLabel(tr("Scan QR code and save the key to another device"), this);
    QFont font2 = m_pScanTipsLabel->font();
    font2.setPixelSize(12);
    m_pScanTipsLabel->setFont(font2);
    m_pScanTipsLabel->setVisible(false);

    // 下一步按钮
    m_pNext = new QPushButton(tr("Next"), this);
    m_pNext->setFixedSize(452, 30);
    connect(m_pNext, &QPushButton::clicked,
            this, &DFMVaultActiveSaveKeyView::slotNextBtnClicked);

    // 布局
    play1 = new QGridLayout();
    play1->setMargin(0);
    play1->addWidget(m_pTipsLabel, 0, 0, 1, 4, Qt::AlignCenter);
    play1->addWidget(m_pButtonBox, 1, 0, 1, 4, Qt::AlignHCenter);
    play1->addWidget(m_pKeyText, 2, 0, 2, 4, Qt::AlignHCenter);


    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabelTitle);
    play->addLayout(play1);
    play->addStretch();
    play->addWidget(m_pNext, 0, Qt::AlignCenter);
}

void DFMVaultActiveSaveKeyView::showEvent(QShowEvent *event)
{
    QString strUserKey = OperatorCenter::getInstance().getUserKey();
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
    if(OperatorCenter::getInstance().createQRCode(strContent, m_pQRCodeImage->width(), m_pQRCodeImage->height(), QRCodePix))
        m_pQRCodeImage->setPixmap(QRCodePix);
    m_pKeyText->appendPlainText(strKeyShow);

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

    play1->addWidget(m_pKeyText, 2, 0, 2, 4);
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
