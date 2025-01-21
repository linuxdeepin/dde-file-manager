// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "eventshandler.h"
#include "dfmplugin_disk_encrypt_global.h"
#include "gui/encryptprogressdialog.h"
#include "gui/unlockpartitiondialog.h"
#include "gui/encryptparamsinputdialog.h"
#include "utils/encryptutils.h"
#include "menu/diskencryptmenuscene.h"

#include <dfm-framework/dpf.h>
#include <dfm-base/utils/finallyutil.h>

#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QSettings>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>

#include <DDialog>

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;
DWIDGET_USE_NAMESPACE;

EventsHandler *EventsHandler::instance()
{
    static EventsHandler ins;
    return &ins;
}

void EventsHandler::bindDaemonSignals()
{
    // FIXME(xust) split the unlock module into another plugin.
    // for unlocking devices in file dialog, this plugin is loaded,
    // which cause when en/decrypt devices, the signal are handled
    // by both file manager and file dialog, so multiple progress
    // dialog and finished dialog are shown.
    // this class is singleton but in different process it's not.
    if (qApp->applicationName() != "dde-file-manager")
        return;

    auto conn = [this](const char *sig, const char *slot) {
        QDBusConnection::systemBus().connect(kDaemonBusName,
                                             kDaemonBusPath,
                                             kDaemonBusIface,
                                             sig,
                                             this,
                                             slot);
    };
    conn("EncryptProgress", SLOT(onEncryptProgress(const QString &, const QString &, double)));
    conn("DecryptProgress", SLOT(onDecryptProgress(const QString &, const QString &, double)));
    conn("InitEncResult", SLOT(onInitEncryptFinished(const QVariantMap &)));
    conn("EncryptResult", SLOT(onEncryptFinished(const QVariantMap &)));
    conn("DecryptResult", SLOT(onDecryptFinished(const QVariantMap &)));
    conn("ChangePassResult", SLOT(onChgPwdFinished(const QVariantMap &)));
    conn("WaitAuthInput", SLOT(onRequestAuthArgs(const QVariantMap &)));
}

void EventsHandler::hookEvents()
{
    dpfHookSequence->follow("dfmplugin_computer", "hook_Device_AcquireDevPwd",
                            this, &EventsHandler::onAcquireDevicePwd);
}

/**
 * @brief EventsHandler::isTaskWorking, if any device is running encrypt, decrypt and change passphrase background
 * @return
 */
bool EventsHandler::isTaskWorking()
{
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    QDBusReply<bool> reply = iface.call("IsTaskRunning");
    return reply.isValid() && reply.value();
}

/**
 * @brief EventsHandler::hasPendingTask, if task files existed in /etc/usec-crypt/
 * @return
 */
bool EventsHandler::hasPendingTask()
{
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    QDBusReply<bool> reply = iface.call("IsTaskEmpty");
    return reply.isValid() && !reply.value();
}

QString EventsHandler::unfinishedDecryptJob()
{
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    QDBusReply<QString> reply = iface.call("PendingDecryptionDevice");
    return reply.value();
}

/**
 * @brief EventsHandler::isUnderOperating, If the device is performing a task in the foreground
 * @param device
 * @return
 */
bool EventsHandler::isUnderOperating(const QString &device)
{
    return encryptDialogs.contains(device)
            || decryptDialogs.contains(device)
            || encryptInputs.contains(device);
}

int EventsHandler::deviceEncryptStatus(const QString &device)
{
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    QDBusReply<int> reply = iface.call("DeviceStatus", device);
    if (reply.isValid())
        return reply.value();
    return -1;
}

void EventsHandler::resumeEncrypt(const QString &device)
{
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    iface.asyncCall("ResumeEncryption", device);
}

void EventsHandler::onInitEncryptFinished(const QVariantMap &result)
{
    QApplication::restoreOverrideCursor();

    auto code = result.value(encrypt_param_keys::kKeyOperationResult).toInt();
    auto dev = result.value(encrypt_param_keys::kKeyDevice).toString();
    auto name = result.value(encrypt_param_keys::kKeyDeviceName).toString();

    if (code == -kRebootRequired) {
        qInfo() << "ask user to reboot machine.";
        requestReboot();
    } else if (code < 0) {
        showPreEncryptError(dev, name, code);
        return;
    }

    autoStartDFM();
}

void EventsHandler::onEncryptFinished(const QVariantMap &result)
{
    QApplication::restoreOverrideCursor();

    auto code = result.value(encrypt_param_keys::kKeyOperationResult).toInt();
    auto dev = result.value(encrypt_param_keys::kKeyDevice).toString();
    auto name = result.value(encrypt_param_keys::kKeyDeviceName).toString();

    // delay delete input dialogs. avoid when new request comes new dialog raises.
    QTimer::singleShot(1000, this, [=] {
        if (encryptInputs.contains(dev))
            encryptInputs.take(dev)->deleteLater();
    });

    QString device = QString("%1(%2)").arg(name).arg(dev.mid(5));
    QString title, msg;
    bool success = false;
    switch (-code) {
    case kUserCancelled:
        ignoreParamRequest();
        return;
    case kSuccess:
    case KErrorRequestExportRecKey:
        title = tr("Encrypt done");
        msg = tr("Device %1 has been encrypted").arg(device);
        success = true;
        break;
    default:
        title = tr("Encrypt failed");
        msg = tr("Device %1 encrypt failed, please see log for more information.(%2)")
                      .arg(device)
                      .arg(code);
        break;
    }

    auto dialog = encryptDialogs.take(dev);
    if (!dialog)
        dialog_utils::showDialog(title, msg, code != 0 ? dialog_utils::kError : dialog_utils::kInfo);
    else {
        auto pos = dialog->geometry().topLeft();
        dialog->showResultPage(success, title, msg);
        if (code == -KErrorRequestExportRecKey) {
            auto recKey = result.value(encrypt_param_keys::kKeyRecoveryKey).toString();
            dialog->setRecoveryKey(recKey, dev);
            dialog->showExportPage();
        }
        dialog->move(pos);
    }

    // delete auto start file.
    auto configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    auto autoStartFilePath = configPath + "/autostart/dfm-reencrypt.desktop";
    int ret = ::remove(autoStartFilePath.toStdString().c_str());
    qInfo() << "autostart file has been removed:" << ret;
}

void EventsHandler::onDecryptFinished(const QVariantMap &result)
{
    QApplication::restoreOverrideCursor();

    auto code = result.value(encrypt_param_keys::kKeyOperationResult).toInt();
    auto dev = result.value(encrypt_param_keys::kKeyDevice).toString();
    auto name = result.value(encrypt_param_keys::kKeyDeviceName).toString();

    if (code == -kRebootRequired) {
        requestReboot();
    } else {
        showDecryptError(dev, name, code);

        // delete auto start file.
        auto configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        auto autoStartFilePath = configPath + "/autostart/dfm-reencrypt.desktop";
        int ret = ::remove(autoStartFilePath.toStdString().c_str());
        qInfo() << "autostart file has been removed:" << ret;
    }
}

void EventsHandler::onChgPwdFinished(const QVariantMap &result)
{
    auto code = result.value(encrypt_param_keys::kKeyOperationResult).toInt();
    auto dev = result.value(encrypt_param_keys::kKeyDevice).toString();
    auto name = result.value(encrypt_param_keys::kKeyDeviceName).toString();

    QApplication::restoreOverrideCursor();
    showChgPwdError(dev, name, code);
}

void EventsHandler::onRequestAuthArgs(const QVariantMap &devInfo)
{
    qApp->restoreOverrideCursor();

    QString devPath = devInfo.value(encrypt_param_keys::kKeyDevice).toString();
    if (devPath.isEmpty()) {
        qWarning() << "invalid encrypt config!" << devInfo;
        return;
    }

    if (encryptInputs.value(devPath, nullptr))
        return;

    QString objPath = "/org/freedesktop/UDisks2/block_devices/" + devPath.mid(5);
    auto blkDev = device_utils::createBlockDevice(objPath);
    auto dlg = new EncryptParamsInputDialog(devInfo, qApp->activeWindow());
    encryptInputs.insert(devPath, dlg);

    connect(dlg, &DDialog::finished, this, [=](auto ret) {
        if (ret != QDialog::Accepted) {
            ignoreParamRequest();
            encryptInputs.take(devPath)->deleteLater();   // also will be deleted when encryption started.
        } else {
            DiskEncryptMenuScene::doReencryptDevice(dlg->getInputs());
        }
    });
    dlg->show();
}

void EventsHandler::ignoreParamRequest()
{
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    iface.asyncCall("IgnoreAuthSetup");
    qInfo() << "ignore param request...";
}

void EventsHandler::onEncryptProgress(const QString &dev, const QString &devName, double progress)
{
    if (!encryptDialogs.contains(dev)) {
        QString device = QString("%1(%2)").arg(devName).arg(dev.mid(5));

        QApplication::restoreOverrideCursor();
        auto dlg = new EncryptProgressDialog(qApp->activeWindow());
        dlg->setText(tr("%1 is under encrypting...").arg(device),
                     tr("The encrypting process may have system lag, please minimize the system operation"));
        encryptDialogs.insert(dev, dlg);
    }
    auto dlg = encryptDialogs.value(dev);
    dlg->updateProgress(progress);
    dlg->show();

    // when start encrypt, delete the inputs widget.
    if (encryptInputs.contains(dev))
        delete encryptInputs.take(dev);
}

void EventsHandler::onDecryptProgress(const QString &dev, const QString &devName, double progress)
{
    if (!decryptDialogs.contains(dev)) {
        QString device = QString("%1(%2)").arg(devName).arg(dev.mid(5));

        QApplication::restoreOverrideCursor();
        auto dlg = new EncryptProgressDialog(qApp->activeWindow());
        dlg->setText(tr("%1 is under decrypting...").arg(device),
                     tr("The decrypting process may have system lag, please minimize the system operation"));
        decryptDialogs.insert(dev, dlg);
    }

    auto dlg = decryptDialogs.value(dev);
    dlg->updateProgress(progress);
    dlg->show();
}

bool EventsHandler::onAcquireDevicePwd(const QString &dev, QString *pwd, bool *cancelled)
{
    if (!pwd || !cancelled)
        return false;

    if (!canUnlock(dev)) {
        *cancelled = true;
        return true;
    }

    int type = device_utils::encKeyType(dev);

    // test tpm
    bool testTPM = (type == kPin || type == kTpm);
    if (testTPM && tpm_utils::checkTPM() != 0) {
        qWarning() << "TPM service is not available.";
        int ret = dialog_utils::showDialog(tr("Error"), tr("TPM status is abnormal, please use the recovery key to unlock it"),
                                           dialog_utils::DialogType::kError);
        // unlock by recovery key.
        if (ret == 0)
            *pwd = acquirePassphraseByRec(dev, *cancelled);

        return true;
    }

    switch (type) {
    case SecKeyType::kPin:
        *pwd = acquirePassphraseByPIN(dev, *cancelled);
        break;
    case SecKeyType::kTpm:
        *pwd = acquirePassphraseByTPM(dev, *cancelled);
        break;
    case SecKeyType::kPwd:
        *pwd = acquirePassphrase(dev, *cancelled);
        break;
    default:
        return false;
    }

    if (pwd->isEmpty() && !*cancelled) {
        QString title;
        if (type == kPin)
            title = tr("Wrong PIN");
        else if (type == kPwd)
            title = tr("Wrong passphrase");
        else
            title = tr("TPM error");

        dialog_utils::showDialog(title, tr("Please use recovery key to unlock device."),
                                 dialog_utils::kInfo);

        *pwd = acquirePassphraseByRec(dev, *cancelled);
    }

    return true;
}

QString EventsHandler::acquirePassphrase(const QString &dev, bool &cancelled)
{
    UnlockPartitionDialog dlg(UnlockPartitionDialog::kPwd);
    int ret = dlg.exec();
    if (ret != 1) {
        cancelled = true;
        return "";
    }
    return dlg.getUnlockKey().second;
}

QString EventsHandler::acquirePassphraseByPIN(const QString &dev, bool &cancelled)
{
    UnlockPartitionDialog dlg(UnlockPartitionDialog::kPin);
    int ret = dlg.exec();
    if (ret != 1) {
        cancelled = true;
        return "";
    }
    auto keys = dlg.getUnlockKey();
    if (keys.first == UnlockPartitionDialog::kPin)
        return tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(dev, keys.second);
    else
        return keys.second;
}

QString EventsHandler::acquirePassphraseByTPM(const QString &dev, bool &)
{
    return tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(dev, "");
}

QString EventsHandler::acquirePassphraseByRec(const QString &dev, bool &cancelled)
{
    UnlockPartitionDialog dlg(UnlockPartitionDialog::kRec);
    int ret = dlg.exec();
    if (ret != 1) {
        cancelled = true;
        return "";
    }
    auto keys = dlg.getUnlockKey();
    return keys.second;
}

void EventsHandler::showPreEncryptError(const QString &dev, const QString &devName, int code)
{
    QString title;
    QString msg;
    QString device = QString("%1(%2)").arg(devName).arg(dev.mid(5));

    bool showError = false;
    switch (-code) {
    case (kSuccess):
        title = tr("Preencrypt done");
        msg = tr("Device %1 has been preencrypt, please reboot to finish encryption.")
                      .arg(device);
        break;
    case kUserCancelled:
        return;
    default:
        title = tr("Preencrypt failed");
        msg = tr("Device %1 preencrypt failed, please see log for more information.(%2)")
                      .arg(device)
                      .arg(code);
        showError = true;
        break;
    }

    dialog_utils::showDialog(title, msg,
                             showError ? dialog_utils::kError : dialog_utils::kInfo);
}

void EventsHandler::showDecryptError(const QString &dev, const QString &devName, int code)
{
    QString title;
    QString msg;
    QString device = QString("%1(%2)").arg(devName).arg(dev.mid(5));

    bool showFailed = true;
    switch (-code) {
    case (kSuccess):
        title = tr("Decrypt done");
        msg = tr("Device %1 has been decrypted").arg(device);
        showFailed = false;
        break;
    case kUserCancelled:
        return;
    case kErrorWrongPassphrase:
        title = tr("Decrypt disk");
        msg = tr("Wrong passpharse or PIN");
        break;
    case kErrorNotFullyEncrypted:
        title = tr("Decrypt failed");
        msg = tr("Device %1 is under encrypting, please decrypt after encryption finished.")
                      .arg(device);
        break;
    default:
        title = tr("Decrypt failed");
        msg = tr("Device %1 Decrypt failed, please see log for more information.(%2)")
                      .arg(device)
                      .arg(code);
        break;
    }

    auto dialog = decryptDialogs.take(dev);
    if (dialog) {
        auto pos = dialog->geometry().topLeft();
        dialog->showResultPage(code == 0, title, msg);
        dialog->move(pos);
    } else {
        dialog_utils::showDialog(title, msg,
                                 showFailed ? dialog_utils::kError : dialog_utils::kInfo);
    }
}

void EventsHandler::showChgPwdError(const QString &dev, const QString &devName, int code)
{
    QString title;
    QString msg;
    QString device = QString("%1(%2)").arg(devName).arg(dev.mid(5));

    int encType = device_utils::encKeyType(dev);
    QString codeType;
    switch (encType) {
    case SecKeyType::kPwd:
        codeType = tr("passphrase");
        break;
    default:
        codeType = tr("PIN");
        break;
    }

    bool showError = false;
    switch (-code) {
    case (kSuccess):
        title = tr("Change %1 done").arg(codeType);
        msg = tr("%1's %2 has been changed").arg(device).arg(codeType);
        break;
    case kUserCancelled:
        return;
    case kErrorChangePassphraseFailed:
        title = tr("Change %1 failed").arg(codeType);
        msg = tr("Wrong %1").arg(codeType);
        showError = true;
        break;
    default:
        title = tr("Change %1 failed").arg(codeType);
        msg = tr("Device %1 change %2 failed, please see log for more information.(%3)")
                      .arg(device)
                      .arg(codeType)
                      .arg(code);
        showError = true;
        break;
    }

    dialog_utils::showDialog(title, msg,
                             showError ? dialog_utils::kError : dialog_utils::kInfo);
}

void EventsHandler::requestReboot()
{
    qWarning() << "reboot is confirmed...";
    QDBusInterface sessMng("com.deepin.SessionManager",
                           "/com/deepin/SessionManager",
                           "com.deepin.SessionManager");
    sessMng.asyncCall("RequestReboot");
}

bool EventsHandler::canUnlock(const QString &device)
{
    if (EventsHandler::instance()->isUnderOperating(device)) {
        return false;
    }

    if (device == unfinishedDecryptJob()) {
        dialog_utils::showDialog(tr("Error"),
                                 tr("Device is not fully decrypted, please finish decryption before access."),
                                 dialog_utils::DialogType::kInfo);
        return false;
    }

    int states = EventsHandler::instance()->deviceEncryptStatus(device);
    if ((states & kStatusOnline) && (states & kStatusEncrypt)) {
        dialog_utils::showDialog(tr("Unlocking device failed"),
                                 tr("Please click the right disk menu \"Continue partition encryption\" to complete partition encryption."),
                                 dialog_utils::DialogType::kError);
        return false;
    }

    return true;
}

void EventsHandler::autoStartDFM()
{
    qInfo() << "autostart is going to added...";
    QDBusInterface sessMng("com.deepin.SessionManager",
                           "/com/deepin/StartManager",
                           "com.deepin.StartManager");
    sessMng.asyncCall("AddAutostart", QString(kReencryptDesktopFile));
}

EventsHandler::EventsHandler(QObject *parent)
    : QObject { parent }
{
}
