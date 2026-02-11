// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultactivefinishedview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/servicemanager.h"
#include "utils/vaultutils.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/vaultconfig.h"

#include "plugins/common/dfmplugin-utils/reportlog/datas/vaultreportdata.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/dpf.h>

#include <DLabel>
#include <DDialog>
#include <DWaterProgress>
#include <DIconButton>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif
#include <DFontSizeManager>

#include <QGridLayout>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QTimer>

Q_DECLARE_METATYPE(const char *)

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveFinishedView::VaultActiveFinishedView(QWidget *parent)
    : VaultBaseView(parent)
{
    initUi();
    initConnect();
}

void VaultActiveFinishedView::initUi()
{
    // 标题
    titleLabel = new DLabel(tr("Encrypt File Vault"), this);
    titleLabel->setAlignment(Qt::AlignHCenter);

    // 加密提示
    tipsLabel = new DLabel(tr("Click 'Encrypt' and input the user password."), this);
    tipsLabel->setAlignment(Qt::AlignHCenter);

    // 加密保险箱图片
    encryVaultImageLabel = new DLabel(this);
    encryVaultImageLabel->setPixmap(QIcon::fromTheme("dfm_vault_active_encrypt").pixmap(98, 88));
    encryVaultImageLabel->setAlignment(Qt::AlignHCenter);

    // 进度条
    waterProgress = new DWaterProgress(this);
    waterProgress->setValue(1);
    waterProgress->setFixedSize(90, 90);
    // 进度条提示
    tipsLabelone = new DLabel(tr("Encrypting..."), this);
    tipsLabelone->setAlignment(Qt::AlignHCenter);

    // 加密完成完成图片
    encryptFinishedImageLabel = new DLabel(this);
    encryptFinishedImageLabel->setAlignment(Qt::AlignHCenter);
    // 加密完成提示
    tipsThree = new DLabel(this);
    tipsThree->setAlignment(Qt::AlignHCenter);
    tipsThree->setWordWrap(true);

    // 加密保险箱按钮
    finishedBtn = new DSuggestButton(tr("Encrypt"), this);
    finishedBtn->setFixedWidth(200);

    // 布局
    widgetOne = new QWidget(this);
    QVBoxLayout *play1 = new QVBoxLayout(widgetOne);
    play1->setContentsMargins(0, 0, 0, 0);
    play1->addWidget(tipsLabel);
    play1->addSpacing(22);
    play1->addWidget(encryVaultImageLabel);

    widgetTow = new QWidget(this);
    QVBoxLayout *play2 = new QVBoxLayout(widgetTow);
    play2->setContentsMargins(0, 22, 0, 0);
    play2->addWidget(waterProgress, 0, Qt::AlignCenter);
    play2->addSpacing(22);
    play2->addWidget(tipsLabelone, 0, Qt::AlignCenter);

    widgetThree = new QWidget(this);
    QVBoxLayout *play3 = new QVBoxLayout(widgetThree);
    play3->setContentsMargins(0, 22, 0, 0);
    play3->addWidget(encryptFinishedImageLabel, 0, Qt::AlignCenter);
    play3->addSpacing(22);
    play3->addWidget(tipsThree);

    QVBoxLayout *m_pLay = new QVBoxLayout(this);
    m_pLay->setContentsMargins(0, 0, 0, 0);
    m_pLay->addWidget(titleLabel);
    m_pLay->addWidget(widgetOne);
    m_pLay->addWidget(widgetTow, 0, Qt::AlignHCenter);
    m_pLay->addWidget(widgetThree);
    m_pLay->addSpacing(40);
    m_pLay->addWidget(finishedBtn, 0, Qt::AlignCenter);

    widgetTow->setVisible(false);
    widgetThree->setVisible(false);

    initUiForSizeMode();

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(titleLabel), AcName::kAcLabelVaultFinishTitle);
    AddATTag(qobject_cast<QWidget *>(tipsLabel), AcName::kAcLabelVaultFinishContent);
    AddATTag(qobject_cast<QWidget *>(encryVaultImageLabel), AcName::kAcLabelVaultFinishVaultImage);
    AddATTag(qobject_cast<QWidget *>(waterProgress), AcName::kAcProgressVaultFinishProgress);
    AddATTag(qobject_cast<QWidget *>(tipsLabelone), AcName::kAcLabelVaultFinishProgressHint);
    AddATTag(qobject_cast<QWidget *>(encryptFinishedImageLabel), AcName::kAcLabelVaultFinishConfirmImage);
    AddATTag(qobject_cast<QWidget *>(tipsThree), AcName::kAcLabelVaultFinishConfirmHint);
    AddATTag(qobject_cast<QWidget *>(finishedBtn), AcName::kAcBtnVaultFinishNext);
#endif
}

void VaultActiveFinishedView::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    DFontSizeManager::instance()->bind(titleLabel, DSizeModeHelper::element(DFontSizeManager::SizeType::T7, DFontSizeManager::SizeType::T5), QFont::Medium);
#else
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::SizeType::T5, QFont::Medium);
#endif
}

void VaultActiveFinishedView::initConnect()
{
    connect(finishedBtn, &DPushButton::clicked,
            this, &VaultActiveFinishedView::slotEncryptVault);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        initUiForSizeMode();
    });
#endif
}

void VaultActiveFinishedView::setFinishedBtnEnabled(bool b)
{
    finishedBtn->setEnabled(b);
    finishedBtn->setText(tr("Encrypt"));
    widgetOne->setVisible(true);
    widgetTow->setVisible(false);
    widgetThree->setVisible(false);
}

void VaultActiveFinishedView::encryptFinished(bool success, const QString &msg)
{
    fmDebug() << "Vault: Encryption finished - success:" << success << "message:" << msg;
    waterProgress->stop();
    widgetOne->setVisible(false);
    widgetTow->setVisible(false);
    widgetThree->setVisible(true);
    finishedBtn->setEnabled(true);
    // 显示右上角关闭按钮
    if (parentWidget()) {
        DDialog *pParent = qobject_cast<DDialog *>(parentWidget()->parentWidget());
        if (pParent) {
            pParent->setCloseButtonVisible(true);
        }
    }

    using namespace dfmplugin_utils;
    if (success) {   // 创建保险箱成功
        fmDebug() << "Vault: Vault creation completed successfully";
        waterProgress->setValue(100);
        encryptFinishedImageLabel->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(100, 100));
        tipsThree->setText(tr("The setup is complete"));
        finishedBtn->setText(tr("OK"));
        VaultHelper::recordTime(kjsonGroupName, kjsonKeyCreateTime);

        // report log
        QVariantMap data;
        data.insert("mode", VaultReportData::kCreated);

        dpfSignalDispatcher->publish("dfmplugin_vault", "signal_ReportLog_Commit", QString("Vault"), data);
    } else {
        fmWarning() << "Vault: Vault creation failed:" << msg;
        encryptFinishedImageLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(100, 100));
        tipsThree->setText(msg);
        finishedBtn->setText(tr("Close"));
    }
}

void VaultActiveFinishedView::setProgressValue(int value)
{
    waterProgress->setValue(value);
}

void VaultActiveFinishedView::slotEncryptVault()
{
    fmDebug() << "Vault: Encrypt vault slot called";
    if (finishedBtn->text() == tr("Encrypt")) {
        fmDebug() << "Vault: Starting vault encryption process";
        // 异步授权
        VaultUtils::instance().showAuthorityDialog(kPolkitVaultCreate);
        connect(&VaultUtils::instance(), &VaultUtils::resultOfAuthority,
                this, &VaultActiveFinishedView::slotCheckAuthorizationFinished,
                Qt::UniqueConnection);
        // 灰化按钮，避免异步时用户再次点击按钮
        finishedBtn->setEnabled(false);
    } else {
        if (finishedBtn->text() == tr("OK")) {
            fmDebug() << "Vault: Switching to vault main page after successful creation";
            // 切换到保险箱主页面
            VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(), VaultHelper::instance()->rootUrl());
            VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
            VaultHelper::recordTime(kjsonGroupName, kjsonKeyLockTime);
        }
        emit accepted();
    }
}

void VaultActiveFinishedView::slotCheckAuthorizationFinished(bool result)
{
    finishedBtn->setEnabled(!result);
    if (result && isVisible() && finishedBtn->text() == tr("Encrypt")) {
        // 进度条
        waterProgress->start();
        widgetOne->setVisible(false);
        widgetTow->setVisible(true);
        widgetThree->setVisible(false);
        emit reqEncryptVault();
    }
}

void VaultActiveFinishedView::slotTimeout()
{
    widgetOne->setVisible(false);
    widgetTow->setVisible(false);
    widgetThree->setVisible(true);
    finishedBtn->setText(tr("OK"));
    finishedBtn->setEnabled(true);
    // 显示右上角关闭按钮
    if (parentWidget()) {
        DDialog *pParent = qobject_cast<DDialog *>(parentWidget()->parentWidget());
        if (pParent) {
            pParent->setCloseButtonVisible(true);
        }
    }
}
