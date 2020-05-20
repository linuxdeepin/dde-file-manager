#include "dfmvaultactivefinishedview.h"
#include "operatorcenter.h"
#include "../../controllers/vaultcontroller.h"
#include "vault/vaultlockmanager.h"
#include "app/define.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <DIconButton>
#include <QDebug>
#include <DWaterProgress>
#include <QThread>

DFMVaultActiveFinishedView::DFMVaultActiveFinishedView(QWidget *parent)
    : QWidget(parent)
    , m_pTips(nullptr)
    , m_pFinishedBtn(nullptr)
    , m_pTips2(nullptr)
{
    // 标题
    QLabel *pLabelTitle = new QLabel(tr("Encrypt File Vault"), this);
    pLabelTitle->setStyleSheet("font: 16pt 'CESI黑体-GB13000'");

    // 提示
    m_pTips = new QLabel(tr("Click 'Encrypt' and input the user password to finish the setup wizard."), this);
    m_pTips2 = new QLabel(tr("It will take several minutes, please wait..."), this);
    m_pTips3 = new QLabel(this);
    m_pTips3->setVisible(false);

    // 加密保险箱图片
    m_pEncryVaultImage = new DIconButton(this);
    m_pEncryVaultImage->setFlat(true);
    m_pEncryVaultImage->setIcon(QIcon::fromTheme("dfm_vault"));
    m_pEncryVaultImage->setIconSize(QSize(210, 210));
    m_pEncryVaultImage->setWindowFlags(Qt::WindowTransparentForInput);
    m_pEncryVaultImage->setFocusPolicy(Qt::NoFocus);
    m_pEncryVaultImage->setMinimumHeight(210);

    // 进度条
    m_pWaterProgress = new DWaterProgress(this);
    m_pWaterProgress->setValue(100);
    m_pWaterProgress->setVisible(false);

    // 加密保险箱按钮
    m_pFinishedBtn = new QPushButton(tr("Encrypt") , this);
    m_pFinishedBtn->setMinimumWidth(450);
    connect(m_pFinishedBtn, &QPushButton::clicked,
            this, &DFMVaultActiveFinishedView::slotEncryptVault);

    play = new QGridLayout();
    play->addWidget(pLabelTitle, 0, 0, 1, 4, Qt::AlignCenter);
    play->addWidget(m_pTips, 1, 0, 1, 4, Qt::AlignCenter);
    play->addWidget(m_pTips2, 2, 0, 1, 4, Qt::AlignCenter);
    play->addWidget(m_pEncryVaultImage, 3, 0, 2, 4, Qt::AlignCenter);

    QVBoxLayout *play2 = new QVBoxLayout(this);
    play2->setMargin(1);
    play2->addLayout(play);
    play2->addStretch();
    play2->addWidget(m_pFinishedBtn, 0, Qt::AlignCenter);

    // cryfs对象
    VaultController * pcryfs = VaultController::getVaultController();
    connect(pcryfs, &VaultController::signalCreateVault,
            this, &DFMVaultActiveFinishedView::slotEncryptComplete);
}

void DFMVaultActiveFinishedView::slotEncryptComplete(int nState)
{
    if(nState == 0){    // 创建保险箱成功
        m_pTips->setVisible(false);
        m_pTips2->setVisible(false);
        m_pWaterProgress->stop();
        m_pWaterProgress->setVisible(false);
        play->removeWidget(m_pWaterProgress);
        m_pEncryVaultImage->setIcon(QIcon::fromTheme("dfm_vault_active_finish"));
        m_pEncryVaultImage->setVisible(true);
        m_pTips3->setText(tr("The setup is complete"));
        m_pFinishedBtn->setText(tr("ok"));
        m_pFinishedBtn->setEnabled(true);
    }else{
        qDebug() << QString(tr("create vault failure, the error code is %1!").arg(nState));
    }
}

void DFMVaultActiveFinishedView::slotEncryptVault()
{
    // 管理员认证
//    if(!OperatorCenter::getInstance().getRootPassword()){
//        return;
//    }
    if (!VaultLockManager::getInstance().checkAuthentication(VAULT_CREATE)){
        return;
    }

    if(m_pFinishedBtn->text() == tr("Encrypt")){
        // 按钮灰化
        m_pFinishedBtn->setEnabled(false);

        // 进度条
        m_pTips->setVisible(false);
        m_pTips2->setVisible(false);
        m_pEncryVaultImage->setVisible(false);
        play->removeWidget(m_pEncryVaultImage);
        play->addWidget(m_pWaterProgress, 3, 0, 2, 4, Qt::AlignCenter);
        m_pWaterProgress->setVisible(true);
        m_pWaterProgress->start();
        play->addWidget(m_pTips3, 5, 0, 1, 4, Qt::AlignHCenter);
        m_pTips3->setVisible(true);
        m_pTips3->setText(tr("Encrypted..."));

        // 调用创建保险箱接口
        // 拿到密码
        QString strPassword = OperatorCenter::getInstance().getSaltAndPasswordClipher();
        VaultController::getVaultController()->createVault(strPassword);

    }else{
        // 切换到保险箱主页面
        emit sigAccepted();
    }
}
