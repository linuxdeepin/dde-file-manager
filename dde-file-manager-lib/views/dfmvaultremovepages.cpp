#include "dfmvaultremovepages.h"
#include "vault/interfaceactivevault.h"
#include "vault/vaultlockmanager.h"
#include "controllers/vaultcontroller.h"
#include "app/define.h"
#include "dfmvaultremoveprogressview.h"
#include "dfmvaultremovebypasswordview.h"
#include "dfmvaultremovebyrecoverykeyview.h"

#include <QFrame>
#include <QRegExpValidator>
#include <QStackedWidget>
#include <QAbstractButton>

#include <DMessageBox>

DWIDGET_USE_NAMESPACE

DFMVaultRemovePages::DFMVaultRemovePages(QWidget *parent)
    : DDialog (parent)
    , m_passwordView(new DFMVaultRemoveByPasswordView(this))
    , m_recoverykeyView(new DFMVaultRemoveByRecoverykeyView(this))
    , m_progressView(new DFMVaultRemoveProgressView(this))
    , m_stackedWidget (new QStackedWidget(this))
{
    this->setTitle(tr("Remove File Vault"));
    this->setIcon(QIcon::fromTheme("dfm_safebox"));
    this->setFixedSize(396, 248);

    m_stackedWidget->addWidget(m_passwordView);
    m_stackedWidget->addWidget(m_recoverykeyView);
    m_stackedWidget->addWidget(m_progressView);
    addContent(m_stackedWidget);

    // 防止点击按钮隐藏界面
    setOnButtonClickedClose(false);

    initConnect();
}

void DFMVaultRemovePages::initConnect()
{
    connect(this, &DFMVaultRemovePages::buttonClicked, this, &DFMVaultRemovePages::onButtonClicked);
    connect(VaultController::getVaultController(), &VaultController::signalLockVault, this, &DFMVaultRemovePages::onLockVault);
    connect(m_progressView, &DFMVaultRemoveProgressView::removeFinished, this, &DFMVaultRemovePages::onVualtRemoveFinish);
}

void DFMVaultRemovePages::showVerifyWidget()
{
    setCloseButtonVisible(true);
    clearButtons();
    setMessage(tr("Once the file vault is removed, the files in it will be permanently deleted. This action cannot be undone, please confirm and continue."));
    QStringList buttonTexts({tr("Cancel"), tr("Use Key"), tr("Remove")});
    addButton(buttonTexts[0], false);
    addButton(buttonTexts[1], false);
    addButton(buttonTexts[2], true);
    getButton(2)->setStyleSheet("color: rgb(255, 85, 0);");
    setDefaultButton(2);
    m_stackedWidget->setCurrentIndex(0);

    // 如果密码提示信息为空，则隐藏提示按钮
    QString strPwdHint("");
    if (InterfaceActiveVault::getPasswordHint(strPwdHint)){
        if (strPwdHint.isEmpty()){
            m_passwordView->setTipsButtonVisible(false);
        } else {
            m_passwordView->setTipsButtonVisible(true);
        }
    }
}

void DFMVaultRemovePages::showRemoveWidget()
{
    setCloseButtonVisible(false);
    clearButtons();
    setMessage(tr("Removing..."));
    addButton(tr("Ok"), true, ButtonType::ButtonNormal);
    getButton(0)->setEnabled(false);
    m_stackedWidget->setCurrentIndex(2);
}

void DFMVaultRemovePages::closeEvent(QCloseEvent *event)
{
    // 重置界面状态
    m_passwordView->clear();
    m_recoverykeyView->clear();
    m_progressView->clear();
    m_bRemoveVault = false;

    event->accept();
}

DFMVaultRemovePages *DFMVaultRemovePages::instance()
{
    static DFMVaultRemovePages s_instance;
    return &s_instance;
}

void DFMVaultRemovePages::setWndPtr(QWidget *wnd)
{
    m_wndptr = wnd;
}

QWidget *DFMVaultRemovePages::getWndPtr() const
{
    return m_wndptr;
}

void DFMVaultRemovePages::showTop()
{
    showVerifyWidget();

    show();
    raise();
}

void DFMVaultRemovePages::onButtonClicked(int index)
{
    switch (index) {
    case 0: //点击取消按钮
        close();
        break;
    case 1:{ // 切换验证方式
        if (m_stackedWidget->currentIndex() == 0){
            getButton(1)->setText(tr("Use Password"));
            m_stackedWidget->setCurrentIndex(1);
        }else {
            getButton(1)->setText(tr("Use Key"));
            m_stackedWidget->setCurrentIndex(0);
        }
    }
        break;
    case 2:{// 删除
        if (m_stackedWidget->currentIndex() == 0){
            // 密码验证
            QString strPwd = m_passwordView->getPassword();
            QString strClipher("");

            if (!InterfaceActiveVault::checkPassword(strPwd, strClipher)){
                m_passwordView->showToolTip(tr("Wrong password"), 3000, DFMVaultRemoveByPasswordView::EN_ToolTip::Warning);
                return;
            }
        }else {
            // 密钥验证
            QString strKey = m_recoverykeyView->getRecoverykey();
            strKey.replace("-", "");
            QString strClipher("");

            if (!InterfaceActiveVault::checkUserKey(strKey, strClipher)){
                m_recoverykeyView->showAlertMessage(tr("Wrong recovery key"));
                return;
            }
        }

        // 管理员权限认证
        if (VaultLockManager::getInstance().checkAuthentication(VAULT_REMOVE)){
            m_bRemoveVault = true;
            // 验证成功，先对保险箱进行上锁
            VaultController::getVaultController()->lockVault();
        }
    }
        break;
    default:
        break;
    }
}

void DFMVaultRemovePages::onLockVault(int state)
{
    if (m_bRemoveVault){
        if (state == 0){
            // 切换至删除界面
            showRemoveWidget();
            emit accepted();

            QString vaultLockPath = VaultController::getVaultController()->vaultLockPath();
            QString vaultUnlockPath = VaultController::getVaultController()->vaultUnlockPath();
            m_progressView->removeVault(vaultLockPath, vaultUnlockPath);
        }else{
            // error tips
            QString errMsg = tr("Remove File Vault failed.%1").arg(VaultController::getErrorInfo(state));
            DMessageBox::information(this, tr("tips"), errMsg);
        }
        m_bRemoveVault = false;
    }
}

void DFMVaultRemovePages::onVualtRemoveFinish(bool result)
{
    if (result){
        this->setMessage(tr("Removed successfully"));
    }else {
        this->setMessage(tr("Failed to remove"));
    }

    this->getButton(0)->setEnabled(true);
}
