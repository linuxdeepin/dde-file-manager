// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultactivefinishedview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/servicemanager.h"
#include "utils/policy/policymanager.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/vaultconfig.h"
#include "events/vaulteventcaller.h"

#include "plugins/common/dfmplugin-utils/reportlog/datas/vaultreportdata.h"
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/dialogmanager.h>
#include "dfm-framework/dpf.h"

#include <DLabel>
#include <DDialog>
#include <DWaterProgress>
#include <DIconButton>

#include <QGridLayout>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QKeyEvent>

Q_DECLARE_METATYPE(const char *)

using namespace PolkitQt1;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;
using namespace dfmplugin_utils;

VaultActiveFinishedView::VaultActiveFinishedView(QWidget *parent)
    : QWidget(parent)
{
    initUi();
    initConnect();
}

VaultActiveFinishedView::~VaultActiveFinishedView()
{
    if (timer && timer->isActive()) {
        timer->stop();
    }
    delete timer;
    if (timerProgress && timerProgress->isActive()) {
        timerProgress->stop();
    }
    delete timerProgress;
}

void VaultActiveFinishedView::initUi()
{
    // 标题
    DLabel *pLabelTitle = new DLabel(tr("Encrypt File Vault"), this);
    QFont font = pLabelTitle->font();
    font.setPixelSize(18);
    pLabelTitle->setFont(font);
    pLabelTitle->setAlignment(Qt::AlignHCenter);

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
    // 进度条提示
    tipsLabelone = new DLabel(tr("Encrypting..."), this);
    tipsLabelone->setAlignment(Qt::AlignHCenter);

    // 加密完成完成图片
    encryptFinishedImageLabel = new DLabel(this);
    encryptFinishedImageLabel->setPixmap(QIcon::fromTheme("dfm_vault_active_finish").pixmap(128, 128));
    encryptFinishedImageLabel->setAlignment(Qt::AlignHCenter);
    // 加密完成提示
    tipsThree = new DLabel(tr("The setup is complete"), this);
    tipsThree->setAlignment(Qt::AlignHCenter);

    // 加密保险箱按钮
    finishedBtn = new DPushButton(tr("Encrypt"), this);

    // 布局
    widgetOne = new QWidget(this);
    QVBoxLayout *play1 = new QVBoxLayout(widgetOne);
    play1->setMargin(0);
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
    play3->setContentsMargins(0, 15, 0, 0);
    play3->addWidget(encryptFinishedImageLabel);
    play3->addSpacing(10);
    play3->addWidget(tipsThree);

    QVBoxLayout *m_pLay = new QVBoxLayout(this);
    m_pLay->setMargin(0);
    m_pLay->addWidget(pLabelTitle);
    m_pLay->addSpacing(10);
    m_pLay->addWidget(widgetOne);
    m_pLay->addWidget(widgetTow, 0, Qt::AlignHCenter);
    m_pLay->addWidget(widgetThree);
    m_pLay->addStretch();
    m_pLay->addWidget(finishedBtn);

    widgetTow->setVisible(false);
    widgetThree->setVisible(false);

    timer = new QTimer(this);
    timerProgress = new QTimer(this);

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(pLabelTitle), AcName::kAcLabelVaultFinishTitle);
    AddATTag(qobject_cast<QWidget *>(tipsLabel), AcName::kAcLabelVaultFinishContent);
    AddATTag(qobject_cast<QWidget *>(encryVaultImageLabel), AcName::kAcLabelVaultFinishVaultImage);
    AddATTag(qobject_cast<QWidget *>(waterProgress), AcName::kAcProgressVaultFinishProgress);
    AddATTag(qobject_cast<QWidget *>(tipsLabelone), AcName::kAcLabelVaultFinishProgressHint);
    AddATTag(qobject_cast<QWidget *>(encryptFinishedImageLabel), AcName::kAcLabelVaultFinishConfirmImage);
    AddATTag(qobject_cast<QWidget *>(tipsThree), AcName::kAcLabelVaultFinishConfirmHint);
    AddATTag(qobject_cast<QWidget *>(finishedBtn), AcName::kAcBtnVaultFinishNext);
#endif
}

void VaultActiveFinishedView::initConnect()
{
    connect(finishedBtn, &DPushButton::clicked,
            this, &VaultActiveFinishedView::slotEncryptVault);
    connect(timer, &QTimer::timeout,
            this, &VaultActiveFinishedView::slotTimeout);
    connect(timerProgress, &QTimer::timeout,
            this, &VaultActiveFinishedView::WaterProgressRising);
}

void VaultActiveFinishedView::setFinishedBtnEnabled(bool b)
{
    finishedBtn->setEnabled(b);
    finishedBtn->setText(tr("Encrypt"));
    widgetOne->setVisible(true);
    widgetTow->setVisible(false);
    widgetThree->setVisible(false);
}

void VaultActiveFinishedView::createVaultFinished(bool success)
{
    timerProgress->stop();
    waterProgress->setValue(100);
    waterProgress->stop();
    if (success) {
        VaultHelper::recordTime(kjsonGroupName, kjsonKeyCreateTime);

        // report log
        QVariantMap data;
        data.insert("mode", VaultReportData::kCreated);
        dpfSignalDispatcher->publish("dfmplugin_vault", "signal_ReportLog_Commit", QString("Vault"), data);
    } else {
        encryptFinishedImageLabel->setPixmap(QIcon::fromTheme("dfm_fail").pixmap(128, 128));
        tipsThree->setText(tr("Encrypt failed!"));
    }
    repaint();
    timer->setSingleShot(true);
    timer->start(500);
}

void VaultActiveFinishedView::slotEncryptVault()
{
    if (finishedBtn->text() == tr("Encrypt")) {
        // 异步授权
        auto ins = Authority::instance();
        ins->checkAuthorization(kPolkitVaultCreate,
                                UnixProcessSubject(getpid()),
                                Authority::AllowUserInteraction);
        connect(ins, &Authority::checkAuthorizationFinished,
                this, &VaultActiveFinishedView::slotCheckAuthorizationFinished);
        // 灰化按钮，避免异步时用户再次点击按钮
        finishedBtn->setEnabled(false);
    } else {
        emit sigAccepted();
        // 切换到保险箱主页面
        VaultEventCaller::sendItemActived(VaultHelper::instance()->currentWindowId(), VaultHelper::instance()->rootUrl());
        VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
        VaultHelper::recordTime(kjsonGroupName, kjsonKeyLockTime);
    }
}

void VaultActiveFinishedView::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultActiveFinishedView::slotCheckAuthorizationFinished);

    if (!isVisible() || result != Authority::Yes || (finishedBtn->text() != tr("Encrypt"))) {
        finishedBtn->setEnabled(true);
        return;
    }

    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage1);

    // 进度条
    waterProgress->start();
    widgetOne->setVisible(false);
    widgetTow->setVisible(true);
    widgetThree->setVisible(false);
    timerProgress->setSingleShot(false);
    timerProgress->start(1000);

    QFutureWatcher<bool> watcher;
    QEventLoop loop;
    connect(&watcher, &QFutureWatcher<bool>::finished, this, [&watcher, &loop]{
        if (watcher.result())
            loop.exit(0);
        else
            loop.exit(-1);
    });
    QFuture<bool> future = QtConcurrent::run([]{
        VaultConfig config;
        const QString sType = config.get(kConfigNodeName, kConfigKeyEncryptionMethod).toString();
        if (sType == kConfigValueMethodTpmWithoutPin || sType == kConfigValueMethodTpmWithPin) {
            if (!OperatorCenter::getInstance()->encryptByTPM(sType)) {
                fmCritical() << "Vault: TPM encrypt failed!";
                return false;
            }
        }

        if (!OperatorCenter::getInstance()->createVault()) {
            fmCritical() << "Vault: Failed to create file vault!";
            return false;
        }
        return true;
    });
    watcher.setFuture(future);

    emit setAllowClose(false);
    bool re = (loop.exec() == 0) ? true : false;
    createVaultFinished(re);
    emit setAllowClose(true);
}

void VaultActiveFinishedView::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage1);
    QWidget::showEvent(event);
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

void VaultActiveFinishedView::WaterProgressRising()
{
    if (waterProgress) {
        int value = waterProgress->value() + 2;
        if (value < 100) {
            waterProgress->setValue(value);
        }
    }
}
