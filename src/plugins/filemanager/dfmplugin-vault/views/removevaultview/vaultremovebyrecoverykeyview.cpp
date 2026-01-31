// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovebyrecoverykeyview.h"
#include "utils/vaulthelper.h"
#include "utils/vaultutils.h"
#include "utils/encryption/operatorcenter.h"

#include <DToolTip>
#include <DFloatingWidget>
#include <DDialog>
#include <DSpinner>
#include <DFrame>
#include <DFileChooserEdit>

#include <QVBoxLayout>
#include <QTimer>
#include <QPlainTextEdit>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QLabel>

#define MAX_KEY_LENGTH (32)   //!凭证最大值，4的倍数

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultRemoveByRecoverykeyView::VaultRemoveByRecoverykeyView(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

VaultRemoveByRecoverykeyView::~VaultRemoveByRecoverykeyView()
{
    if (tooltip) {
        tooltip->deleteLater();
    }
}

void VaultRemoveByRecoverykeyView::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    DFrame *bckFrame = new DFrame(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(bckFrame);
    layout->addWidget(bckFrame);

    if (VaultHelper::instance()->getVaultVersion()) {
        QLabel *label = new QLabel(tr("Delete file vault with recovery key"), this);

        filePathEdit = new DFileChooserEdit(this);
        filePathEdit->lineEdit()->setPlaceholderText(tr("Select Key File"));
        filePathEdit->lineEdit()->setReadOnly(true);
        filePathEdit->lineEdit()->setClearButtonEnabled(false);
        filePathEdit->setDirectoryUrl(QDir::homePath());
        filePathEdit->setFileMode(QFileDialog::ExistingFiles);
        filePathEdit->setNameFilters({ QString("KEY file(*.key)") });

        contentLayout->addWidget(label);
        contentLayout->addWidget(filePathEdit);
    } else {
        keyEdit = new QPlainTextEdit(this);
        keyEdit->setFrameShape(QFrame::NoFrame);
        keyEdit->setPlaceholderText(tr("Input the 32-digit recovery key"));
        keyEdit->installEventFilter(this);
        contentLayout->addWidget(keyEdit);

        connect(keyEdit, &QPlainTextEdit::textChanged, this, &VaultRemoveByRecoverykeyView::onRecoveryKeyChanged);
    }

    // 加载动画（放在窗口中间，覆盖在内容上方）
    spinner = new DSpinner(this);
    spinner->setFixedSize(48, 48);
    spinner->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    spinner->setFocusPolicy(Qt::NoFocus);
    spinner->hide();
}

QString VaultRemoveByRecoverykeyView::getRecoverykey()
{
    if (!keyEdit)
        return {};

    QString strKey = keyEdit->toPlainText();
    return strKey.replace("-", "");
}

void VaultRemoveByRecoverykeyView::showAlertMessage(const QString &text, int duration)
{
    fmDebug() << "Vault: Showing alert message:" << text << "duration:" << duration;

    if (!tooltip) {
        tooltip = new DToolTip(text);
        tooltip->setObjectName("AlertTooltip");
        tooltip->setForegroundRole(DPalette::TextWarning);
        tooltip->setWordWrap(true);

        floatWidget = new DFloatingWidget;
        floatWidget->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        floatWidget->setBackgroundRole(QPalette::ToolTipBase);
        floatWidget->setWidget(tooltip);
    }

    floatWidget->setParent(parentWidget());

    tooltip->setText(text);
    if (floatWidget->parent()) {
        floatWidget->setGeometry(0, 25, 68, 26);
        floatWidget->show();
        floatWidget->adjustSize();
        floatWidget->raise();
    }

    if (duration < 0) {
        fmDebug() << "Vault: Alert message set to persistent display";
        return;
    }

    QTimer::singleShot(duration, floatWidget, [=] {
        floatWidget->close();
    });
}

QStringList VaultRemoveByRecoverykeyView::btnText() const
{
    return { tr("Cancel"), tr("Delete") };
}

QString VaultRemoveByRecoverykeyView::titleText() const
{
    return tr("Delete File Vault");
}

void VaultRemoveByRecoverykeyView::buttonClicked(int index, const QString &text)
{
    Q_UNUSED(text)

    switch (index) {
    case 0: {   // cancel
        fmDebug() << "Vault: Cancel button clicked, closing dialog";
        emit sigCloseDialog();
    } break;
    case 1: {   // ok
        fmInfo() << "Vault: Delete button clicked, validating recovery key";
        // Show loading animation
        spinner->move((width() - spinner->width()) / 2, (height() - spinner->height()) / 2);
        spinner->show();
        spinner->raise();
        spinner->start();

        if (VaultHelper::instance()->getVaultVersion())
            checkRecoveryKeyV2();
        else
            checkRecoveryKeyV1();
    } break;
    default:
        break;
    }
}

void VaultRemoveByRecoverykeyView::onRecoveryKeyChanged()
{
    QString key = keyEdit->toPlainText();
    int length = key.length();
    int maxLength = MAX_KEY_LENGTH + 7;
    fmDebug() << "Vault: Recovery key changed - length:" << length << "max allowed:" << maxLength;

    //! 限制密钥输入框只能输入数字、字母、以及+/-
    QRegularExpression rx("[a-zA-Z0-9-+/]+");
    QString res;
    QRegularExpressionMatch match;
    int pos = 0;

    while ((match = rx.match(key, pos)).hasMatch()) {
        res += match.captured(0);
        pos = match.capturedEnd();
    }

    key = res;

    keyEdit->blockSignals(true);
    //! 限制输入的最大长度
    if (length > maxLength) {
        int position = keyEdit->textCursor().position();
        QTextCursor textCursor = keyEdit->textCursor();
        key.remove(position - (length - maxLength), length - maxLength);
        keyEdit->setPlainText(key);
        textCursor.setPosition(position - (length - maxLength));
        keyEdit->setTextCursor(textCursor);
        fmWarning() << "Vault: Recovery key truncated from" << length << "to" << maxLength << "characters";

        keyEdit->blockSignals(false);
        return;
    }

    int position = afterRecoveryKeyChanged(key);
    keyEdit->setPlainText(key);

    QTextCursor textCursor = keyEdit->textCursor();
    textCursor.setPosition(position);
    keyEdit->setTextCursor(textCursor);
    keyEdit->blockSignals(false);
}

void VaultRemoveByRecoverykeyView::slotCheckAuthorizationFinished(bool result)
{
    fmInfo() << "Vault: Authorization check finished with result:" << result;

    disconnect(&VaultUtils::instance(), &VaultUtils::resultOfAuthority,
               this, &VaultRemoveByRecoverykeyView::slotCheckAuthorizationFinished);

    if (!result) {
        fmWarning() << "Vault: Authorization failed, operation cancelled";
        return;
    }

    fmDebug() << "Vault: Authorization successful, attempting to lock vault";
    if (!VaultHelper::instance()->lockVault(false)) {
        fmCritical() << "Vault: Failed to lock vault for removal";
        QString errMsg = tr("Failed to delete file vault");
        DDialog dialog(this);
        dialog.setIcon(QIcon::fromTheme("dde-file-manager"));
        dialog.setTitle(errMsg);
        dialog.addButton(tr("OK"), true, DDialog::ButtonRecommend);
        fmDebug() << "Vault: Showing error dialog for lock failure";
        dialog.exec();
        return;
    }

    fmDebug() << "Vault: Vault locked successfully, proceeding to removal progress";
    QTimer::singleShot(0, this, [this]() {
        emit signalJump(RemoveWidgetType::kRemoveProgressWidget);
    });
}

int VaultRemoveByRecoverykeyView::afterRecoveryKeyChanged(QString &str)
{
    if (str.isEmpty()) {
        fmDebug() << "Vault: Recovery key is empty, returning position -1";
        return -1;
    }

    int location = keyEdit->textCursor().position();   //! 计算当前光标位置
    int srcLength = str.length();   //! 用于计算原有字符串中的“-”数量
    //!清除所有的“-”
    str.replace("-", "");
    int minusNumber = srcLength - str.length();   //! 原有字符串中的“-”数量

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

    //!计算添加“-”后，重新计算下光标的位置，
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

void VaultRemoveByRecoverykeyView::checkRecoveryKeyV1()
{
    const QString key = getRecoverykey();
    keyEdit->setEnabled(false);

    QFuture<bool> future = QtConcurrent::run([this, key]() {
        return validateRecoveryKeyV1(key);
    });

    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
        bool isValid = watcher->result();
        watcher->deleteLater();
        handleRecoveryKeyV1ValidationResult(isValid);
    });
    watcher->setFuture(future);
}

void VaultRemoveByRecoverykeyView::checkRecoveryKeyV2()
{
    const auto &file = filePathEdit->text();
    if (file.isEmpty()) {
        spinner->stop();
        spinner->hide();
        filePathEdit->showAlertMessage(tr("Please select a recovery key file"));
        return;
    }

    QFuture<bool> future = QtConcurrent::run([this, file]() {
        return validateRecoveryKeyFile(file);
    });

    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
        auto isValid = watcher->result();
        watcher->deleteLater();
        handleRecoveryKeyFileValidationResult(isValid);
    });
    watcher->setFuture(future);
}

bool VaultRemoveByRecoverykeyView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(watched);
        if (edit == keyEdit) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

            //! 过滤换行操作以及“-”
            if (keyEvent->key() == Qt::Key_Enter
                || keyEvent->key() == Qt::Key_Return
                || keyEvent->key() == Qt::Key_Minus) {
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

bool VaultRemoveByRecoverykeyView::validateRecoveryKeyV1(const QString &key)
{
    OperatorCenter *operatorCenter = OperatorCenter::getInstance();
    QString cipher;
    bool isValid = false;

    if (operatorCenter->isNewVaultVersion()) {
        // 新版本：直接使用恢复密钥（32字符字符串）验证
        if (key.length() != 32) {
            fmWarning() << "Vault: Invalid recovery key format, expected 32 characters, got" << key.length();
            return false;
        }
        isValid = operatorCenter->checkPassword(key, cipher);
        fmDebug() << "Vault: New version recovery key validation result:" << isValid;
    } else {
        // 旧版本：使用RSA用户密钥验证
        isValid = operatorCenter->checkUserKey(key, cipher);
        fmDebug() << "Vault: Old version user key validation result:" << isValid;
    }

    return isValid;
}

void VaultRemoveByRecoverykeyView::handleRecoveryKeyV1ValidationResult(bool isValid)
{
    // Hide loading animation
    spinner->stop();
    spinner->hide();
    keyEdit->setEnabled(true);

    if (!isValid) {
        fmWarning() << "Vault: Recovery key validation failed";
        showAlertMessage(tr("Wrong recovery key"));
        return;
    }

    fmInfo() << "Vault: Recovery key validated successfully, requesting authorization";
    VaultUtils::instance().showAuthorityDialog(kPolkitVaultRemove);
    connect(&VaultUtils::instance(), &VaultUtils::resultOfAuthority,
            this, &VaultRemoveByRecoverykeyView::slotCheckAuthorizationFinished);
}

bool VaultRemoveByRecoverykeyView::validateRecoveryKeyFile(const QString &file)
{
    QString password;
    return OperatorCenter::getInstance()->verificationRetrievePassword(file, password);
}

void VaultRemoveByRecoverykeyView::handleRecoveryKeyFileValidationResult(bool isValid)
{
    // Hide loading animation
    spinner->stop();
    spinner->hide();

    if (!isValid) {
        fmWarning() << "Vault: Recovery key file validation failed";
        filePathEdit->showAlertMessage(tr("Invalid recovery key file"));
        return;
    }

    fmInfo() << "Vault: Recovery key file validated successfully, requesting authorization";
    VaultUtils::instance().showAuthorityDialog(kPolkitVaultRemove);
    connect(&VaultUtils::instance(), &VaultUtils::resultOfAuthority,
            this, &VaultRemoveByRecoverykeyView::slotCheckAuthorizationFinished);
}
