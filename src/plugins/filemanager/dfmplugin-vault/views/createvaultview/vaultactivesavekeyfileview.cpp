// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/radioframe.h"
#include "vaultactivesavekeyfileview.h"
#include "utils/vaultdefine.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaulthelper.h"
#include "utils/vaultautolock.h"
#include "dfmplugin_vault_global.h"
#include "utils/pathmanager.h"

#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/event/event.h>

#include <DPalette>
#include <DFontSizeManager>
#include <DFileDialog>
#include <DLabel>
#include <DFileChooserEdit>
#include <DFrame>
#include <DSpinner>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <DPushButton>
#include <DDialog>
#include <QLabel>
#include <QIcon>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainterPath>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveSaveKeyFileView::VaultActiveSaveKeyFileView(QWidget *parent)
    : VaultBaseView(parent)
{
    initUI();
    initConnect();
}

void VaultActiveSaveKeyFileView::setEncryptInfo(EncryptInfo &info)
{
    info.keyPath = selectfileSavePathEdit->text();
}

void VaultActiveSaveKeyFileView::setNextButtonText(const QString &text)
{
    if (nextBtn) {
        nextBtn->setText(text);
    }
}

void VaultActiveSaveKeyFileView::setOldPasswordSchemeMigrationMode(bool enabled)
{
    isOldPasswordSchemeMigrationMode = enabled;
}

void VaultActiveSaveKeyFileView::initUI()
{
    titleLabel = new DLabel(this);
    titleLabel->setForegroundRole(DPalette::TextTitle);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setText(tr("Recovery key file"));

    hintMsg = new DLabel(this);
    DFontSizeManager::instance()->bind(hintMsg, DFontSizeManager::T7, QFont::Normal);
    hintMsg->setForegroundRole(DPalette::TextTips);
    hintMsg->setWordWrap(true);
    hintMsg->setAlignment(Qt::AlignCenter);
    hintMsg->setText(tr("Save the recovery key file and keep it in a safe place"));
    otherPathLabel = new DLabel(this);
    DFontSizeManager::instance()->bind(otherPathLabel, DFontSizeManager::T8, QFont::Medium);
    otherPathLabel->setForegroundRole(DPalette::ButtonText);
    otherPathLabel->setText(tr("It is recommended to save it to external storage devices such as a USB drive"));

    otherRadioBtnHitMsg = new DLabel(tr("No permission, please reselect"), this);
    otherRadioBtnHitMsg->hide();
    DFontSizeManager::instance()->bind(otherRadioBtnHitMsg, DFontSizeManager::T9, QFont::Normal);
    otherRadioBtnHitMsg->setForegroundRole(DPalette::TextWarning);

    selectfileSavePathEdit = new DFileChooserEdit(this);
    selectfileSavePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
    selectfileSavePathEdit->lineEdit()->setReadOnly(true);
    selectfileSavePathEdit->lineEdit()->setClearButtonEnabled(false);
    filedialog = new DFileDialog(this, QDir::homePath(), QString("recoveryKey.key"));
    filedialog->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    filedialog->setDefaultSuffix(QString("key"));
    selectfileSavePathEdit->setDirectoryUrl(QDir::homePath());
    selectfileSavePathEdit->setFileMode(QFileDialog::AnyFile);
    selectfileSavePathEdit->setNameFilters({ "KEY file(*.key)" });
    selectfileSavePathEdit->setFileDialog(filedialog);
    selectfileSavePathEdit->setEnabled(true);

    nextBtn = new DSuggestButton(tr("Next"), this);
    nextBtn->setFixedWidth(200);
    nextBtn->setEnabled(false);

    RadioFrame *frame = new RadioFrame;
    RadioFrame *frame1 = new RadioFrame;

    DFrame *line = new DFrame(this);
    line->setObjectName(QString("line"));
    line->setFixedHeight(1);
    line->installEventFilter(this);

    QHBoxLayout *layout3 = new QHBoxLayout();
    layout3->setContentsMargins(0, 0, 0, 0);
    layout3->addWidget(otherPathLabel);
    layout3->addWidget(otherRadioBtnHitMsg);
    layout3->addStretch(1);

    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->setContentsMargins(8, 5, 8, 5);
    layout4->addLayout(layout3);

    QHBoxLayout *layout5 = new QHBoxLayout;
    layout5->setContentsMargins(10, 10, 10, 10);
    layout5->addWidget(selectfileSavePathEdit);

    QVBoxLayout *vlayout5 = new QVBoxLayout(frame1);
    vlayout5->setContentsMargins(0, 0, 0, 0);
    vlayout5->setSpacing(0);
    vlayout5->addLayout(layout4);
    vlayout5->addWidget(line);
    vlayout5->addLayout(layout5);

    QVBoxLayout *vlayout3 = new QVBoxLayout;
    vlayout3->setContentsMargins(20, 0, 20, 0);
    vlayout3->addWidget(frame, 1);
    vlayout3->addWidget(frame1, 1);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->setContentsMargins(0, 0, 0, 0);
    vlayout1->addWidget(titleLabel);
    vlayout1->addWidget(hintMsg);
    vlayout1->addStretch(1);
    vlayout1->addLayout(vlayout3);
    vlayout1->addStretch(3);
    vlayout1->addWidget(nextBtn, 0, Qt::AlignCenter);

    setLayout(vlayout1);
    initUiForSizeMode();

    spinner = new DSpinner(this);
    spinner->setFixedSize(48, 48);
    spinner->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    spinner->setFocusPolicy(Qt::NoFocus);
    spinner->hide();

    oldPasswordSchemeMigrationWatcher = new QFutureWatcher<OldPasswordSchemeMigrationResult>(this);
    connect(oldPasswordSchemeMigrationWatcher, &QFutureWatcher<OldPasswordSchemeMigrationResult>::finished,
            this, &VaultActiveSaveKeyFileView::onOldPasswordSchemeMigrationFinished);

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(titleLabel), AcName::kAcLabelVaultSaveKeyTitle);
    AddATTag(qobject_cast<QWidget *>(hintMsg), AcName::kAcLabelVaultSaveKeyContent);
    AddATTag(qobject_cast<QWidget *>(selectfileSavePathEdit), AcName::kAcEditVaultSaveKeyPath);
    AddATTag(qobject_cast<QWidget *>(nextBtn), AcName::kAcBtnVaultSaveKeyNext);
#endif
}

void VaultActiveSaveKeyFileView::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    DFontSizeManager::instance()->bind(titleLabel, DSizeModeHelper::element(DFontSizeManager::SizeType::T7, DFontSizeManager::SizeType::T5), QFont::Medium);
#else
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::SizeType::T5, QFont::Medium);
#endif
}

void VaultActiveSaveKeyFileView::initConnect()
{
    connect(selectfileSavePathEdit, &DFileChooserEdit::fileChoosed, this, &VaultActiveSaveKeyFileView::slotChangeEdit);
    connect(filedialog, &DFileDialog::fileSelected, this, &VaultActiveSaveKeyFileView::slotSelectCurrentFile);
    connect(nextBtn, &DPushButton::clicked,
            this, &VaultActiveSaveKeyFileView::slotNextBtnClicked);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        initUiForSizeMode();
    });
#endif
}

void VaultActiveSaveKeyFileView::slotNextBtnClicked()
{
    QString recoveryKey = OperatorCenter::getInstance()->getRecoveryKey();

    if (recoveryKey.isEmpty()) {
        fmCritical() << "Vault: Recovery key is empty for new vault, this should not happen";
        DialogManager::instance()->showErrorDialog(tr("Vault"), tr("Recovery key is not available. Please try again."));
        return;
    }

    QString path = selectfileSavePathEdit->text();
    if (path.isEmpty()) {
        fmWarning() << "Vault: Save path is empty";
        DialogManager::instance()->showMessageDialog(tr("Vault"), tr("Please select a path to save the recovery key."));
        return;
    }

    QFileInfo fileInfo(path);
    if (fileInfo.isDir() || fileInfo.isRelative()) {
        if (!path.endsWith(".key")) {
            if (!path.endsWith("/")) {
                path += "/";
            }
            path += "recoveryKey.key";
        }
    }

    if (!OperatorCenter::getInstance()->saveKey(recoveryKey, path).result) {
        fmWarning() << "Vault: Failed to save recovery key to file:" << path;
        DialogManager::instance()->showErrorDialog(tr("Vault"), tr("Failed to save recovery key. Please check the path and try again."));
        return;
    }

    if (isOldPasswordSchemeMigrationMode) {
        QString oldPassword = OperatorCenter::getInstance()->getPendingOldPasswordSchemeMigrationPassword();
        if (oldPassword.isEmpty()) {
            fmWarning() << "Vault: Pending old password scheme migration password is empty";
            DialogManager::instance()->showErrorDialog(tr("Vault"), tr("Failed to start migration. Please try again."));
            return;
        }

        spinner->move((width() - spinner->width()) / 2, (height() - spinner->height()) / 2);
        spinner->show();
        spinner->raise();
        spinner->start();
        nextBtn->setEnabled(false);
        selectfileSavePathEdit->setEnabled(false);

        QFuture<OldPasswordSchemeMigrationResult> future = QtConcurrent::run([oldPassword, recoveryKey]() -> OldPasswordSchemeMigrationResult {
            OldPasswordSchemeMigrationResult result;
            result.unlocked = false;
            QString outRecoveryKey;
            result.success = OperatorCenter::getInstance()->upgradeOldVaultByPassword(oldPassword, outRecoveryKey);
            if (result.success && outRecoveryKey != recoveryKey) {
                OperatorCenter::getInstance()->setRecoveryKey(outRecoveryKey);
            }
            if (result.success) {
                QString masterKey;
                if (InterfaceActiveVault::checkPassword(oldPassword, masterKey)) {
                    result.unlocked = VaultHelper::instance()->unlockVault(masterKey);
                }
            }
            return result;
        });
        oldPasswordSchemeMigrationWatcher->setFuture(future);
    } else {
        emit sigAccepted();
    }
}

void VaultActiveSaveKeyFileView::onOldPasswordSchemeMigrationFinished()
{
    OldPasswordSchemeMigrationResult result = oldPasswordSchemeMigrationWatcher->result();

    spinner->stop();
    spinner->hide();
    nextBtn->setEnabled(true);
    selectfileSavePathEdit->setEnabled(true);

    if (!result.success) {
        DialogManager::instance()->showErrorDialog(tr("Vault"), tr("Failed to upgrade vault. Please try again."));
        return;
    }

    if (result.unlocked) {
        VaultAutoLock::instance()->slotUnlockVault(static_cast<int>(ErrorCode::kSuccess));
    }

    DDialog successDialog(this);
    successDialog.setTitle(tr("Success"));

    QFrame *contentFrame = new QFrame(&successDialog);
    QVBoxLayout *layout = new QVBoxLayout(contentFrame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    QLabel *iconLabel = new QLabel(contentFrame);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(64, 64));

    QLabel *textLabel = new QLabel(contentFrame);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);
    textLabel->setText(tr("Vault upgraded successfully. Please keep your recovery key safe."));

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);

    successDialog.setMessage("");
    successDialog.addContent(contentFrame);
    successDialog.addButton(tr("Enter Vault", "button"), true, DDialog::ButtonRecommend);

    int ret = successDialog.exec();

    if (ret == 0) {
        if (result.unlocked && VaultHelper::instance()->state(PathManager::vaultLockPath()) == VaultState::kUnlocked) {
            VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(),
                                                     VaultHelper::instance()->rootUrl());
            VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
        }
    }

    emit sigAccepted();
}

void VaultActiveSaveKeyFileView::slotChangeEdit(const QString &fileName)
{
    fmDebug() << "Vault: File chooser edit changed, fileName:" << fileName;

    if (fileName.isEmpty()) {
        nextBtn->setEnabled(false);
        return;
    }

    QFileInfo fileInfo(fileName);
    QString path = fileInfo.isDir() ? fileName : fileInfo.absolutePath();
    QFile file(path);
    QFileDevice::Permissions ps = file.permissions();
    auto temp = ps & QFileDevice::WriteUser;
    if (temp != QFileDevice::WriteUser) {
        nextBtn->setEnabled(false);
        otherRadioBtnHitMsg->show();
        fmWarning() << "Vault: No write permission for path:" << path;
    } else if (!fileName.isEmpty()) {
        otherRadioBtnHitMsg->hide();
        nextBtn->setEnabled(true);
    }
}

void VaultActiveSaveKeyFileView::slotSelectCurrentFile(const QString &file)
{
    QFileInfo fileInfo(file);
    if (fileInfo.isDir()) {
        selectfileSavePathEdit->fileDialog()->selectFile(QString("recoveryKey.key"));
    } else if (!file.endsWith(QString(".key"))) {
        selectfileSavePathEdit->fileDialog()->selectFile(file + QString(".key"));
    }
}

void VaultActiveSaveKeyFileView::showEvent(QShowEvent *event)
{
    selectfileSavePathEdit->clear();
    otherRadioBtnHitMsg->hide();
    QWidget::showEvent(event);
}

bool VaultActiveSaveKeyFileView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched->objectName() == QString("line") && event->type() == QEvent::Paint) {
        QFrame *frame = static_cast<QFrame *>(watched);
        QPainter painter(frame);
        QPalette palette = this->palette();
        painter.setBrush(palette.window());

        painter.setPen(Qt::transparent);
        QRect rect = this->rect();
        rect.setWidth(rect.width() - 1);
        rect.setHeight(rect.height() - 1);
        painter.drawRoundedRect(rect, 0, 0);
        {
            QPainterPath painterPath;
            painterPath.addRoundedRect(rect, 0, 0);
            painter.drawPath(painterPath);
        }
    }

    return QWidget::eventFilter(watched, event);
}
