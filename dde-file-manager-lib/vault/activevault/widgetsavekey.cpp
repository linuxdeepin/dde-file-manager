#include "widgetsavekey.h"
#include "operatorcenter.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include <QHBoxLayout>
#include <DButtonBox>



WidgetSaveKey::WidgetSaveKey(QWidget *parent)
    : QWidget(parent)
    , m_pTipsLabel(nullptr)
    , m_pQRCodeImage(nullptr)
    , m_pNext(nullptr)
{
    // 标题名
    QLabel *pLabelTitle = new QLabel(tr("Recovery Key"), this);
    pLabelTitle->setStyleSheet("font: 16pt 'CESI黑体-GB13000'");

    // 提示标签
    m_pTipsLabel = new QLabel(tr("Generate a recovery key in case that you forget the password"), this);

    DButtonBox *m_pButtonBox = new DButtonBox(this);
    // 密钥
    m_pKeyBtn = new DButtonBoxButton(tr("Key"), this);
    m_pKeyBtn->setFocus();
    connect(m_pKeyBtn, &DButtonBoxButton::clicked,
            this, &WidgetSaveKey::slotKeyBtnClicked);
    m_pKeyText = new QTextEdit(this);
    m_pKeyText->setReadOnly(true);

    // 二维码
    m_pQRCodeBtn = new DButtonBoxButton(tr("QR code"), this);
    connect(m_pQRCodeBtn, &DButtonBoxButton::clicked,
            this, &WidgetSaveKey::slotQRCodeBtnClicked);
    m_pQRCodeImage = new QLabel(this);
    m_pQRCodeImage->setFixedSize(150, 150);
    m_pQRCodeImage->setVisible(false);

    QList<DButtonBoxButton*> lstBtn;
    lstBtn.push_back(m_pKeyBtn);
    lstBtn.push_back(m_pQRCodeBtn);
    m_pButtonBox->setButtonList(lstBtn, true);

    // 扫描提示
    m_pScanTipsLabel = new QLabel(tr("Scan QR code and save the key to another device"), this);
    m_pScanTipsLabel->setVisible(false);

    // 下一步按钮
    m_pNext = new QPushButton(tr("Next"), this);
    m_pNext->setMinimumWidth(450);
    connect(m_pNext, &QPushButton::clicked,
            this, &WidgetSaveKey::slotNextBtnClicked);

    // 布局
    play = new QGridLayout();
    play->addWidget(pLabelTitle, 0, 0, 1, 4, Qt::AlignCenter);
    play->addWidget(m_pTipsLabel, 1, 0, 1, 4, Qt::AlignCenter);
    play->addWidget(m_pButtonBox, 2, 0, 1, 4);
    play->addWidget(m_pKeyText, 3, 0, 2, 4);


    QVBoxLayout *play2 = new QVBoxLayout(this);
    play2->setMargin(1);
    play2->addLayout(play);
    play2->addStretch();
    play2->addWidget(m_pNext, 0, Qt::AlignCenter);
}

void WidgetSaveKey::showEvent(QShowEvent *event)
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
    QString strContent = QString("文件保险箱回复密钥：%1").arg(strKeyShow);
    QPixmap QRCodePix;
    if(OperatorCenter::getInstance().createQRCode(strContent, m_pQRCodeImage->width(), m_pQRCodeImage->height(), QRCodePix))
        m_pQRCodeImage->setPixmap(QRCodePix);
    m_pKeyText->setText(strKeyShow);


    QWidget::showEvent(event);
}

void WidgetSaveKey::slotNextBtnClicked()
{
    emit sigAccepted();
}

void WidgetSaveKey::slotKeyBtnClicked()
{

    play->removeWidget(m_pQRCodeImage);
    m_pQRCodeImage->setVisible(false);
    play->removeWidget(m_pScanTipsLabel);
    m_pScanTipsLabel->setVisible(false);

    play->addWidget(m_pKeyText, 3, 0, 2, 4);
    m_pKeyText->setVisible(true);
}

void WidgetSaveKey::slotQRCodeBtnClicked()
{
    m_pKeyText->setVisible(false);
    play->removeWidget(m_pKeyText);

    play->addWidget(m_pQRCodeImage, 3, 0, 2, 4, Qt::AlignHCenter);
    m_pQRCodeImage->setVisible(true);
    play->addWidget(m_pScanTipsLabel, 5, 0, 1, 4, Qt::AlignHCenter);
    m_pScanTipsLabel->setVisible(true);
}
