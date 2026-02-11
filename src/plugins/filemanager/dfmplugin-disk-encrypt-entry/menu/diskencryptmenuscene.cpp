// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmplugin_disk_encrypt_global.h"
#include "diskencryptmenuscene.h"
#include "globaltypesdefine.h"
#include "gui/decryptparamsinputdialog.h"
#include "gui/chgpassphrasedialog.h"
#include "gui/unlockpartitiondialog.h"
#include "events/eventshandler.h"
#include "utils/encryptutils.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-mount/dmount.h>

#include <QDebug>
#include <QMenu>
#include <QProcess>
#include <QFile>
#include <QStringList>
#include <QDBusInterface>
#include <QDBusReply>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDBusUnixFileDescriptor>
#include <QDataStream>

#include <ddialog.h>
#include <dconfig.h>

#include <fstab.h>
#include <unistd.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_diskenc;
using namespace disk_encrypt;

static constexpr char kActIDEncrypt[] { "de_0_encrypt" };
static constexpr char kActIDResumeEncrypt[] { "de_0_resumeEncrypt" };
static constexpr char kActIDUnlock[] { "de_0_unlock" };
static constexpr char kActIDDecrypt[] { "de_1_decrypt" };
static constexpr char kActIDResumeDecrypt[] { "de_1_resumeDecrypt" };
static constexpr char kActIDChangePwd[] { "de_2_changePwd" };

static constexpr char kOverleyEncPrefix[] { "usec-overlay-" };

DiskEncryptMenuScene::DiskEncryptMenuScene(QObject *parent)
    : AbstractMenuScene(parent)
{
}

dfmbase::AbstractMenuScene *DiskEncryptMenuCreator::create()
{
    return new DiskEncryptMenuScene();
}

QString DiskEncryptMenuScene::name() const
{
    return DiskEncryptMenuCreator::name();
}

bool DiskEncryptMenuScene::initialize(const QVariantHash &params)
{
    if (!config_utils::enableEncrypt()) {
        fmInfo() << "Partition encryption feature is disabled";
        return false;
    }

    QList<QUrl> selectedItems = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (selectedItems.isEmpty()) {
        fmDebug() << "No selected items found";
        return false;
    }

    auto selectedItem = selectedItems.first();
    if (!selectedItem.path().endsWith("blockdev")) {
        fmDebug() << "Selected item is not a block device";
        return false;
    }

    QSharedPointer<FileInfo> info = InfoFactory::create<FileInfo>(selectedItem);
    if (!info) {
        fmWarning() << "Failed to create file info for selected item";
        return false;
    }
    info->refresh();

    selectedItemInfo = info->extraProperties();
    auto device = selectedItemInfo.value("Device", "").toString();
    if (device.isEmpty()) {
        fmWarning() << "Device path is empty";
        return false;
    }

    const QString &idType = selectedItemInfo.value("IdType").toString();
    auto devSymlinks = selectedItemInfo.value("Symlinks").toStringList();
    bool isOverlay = std::any_of(devSymlinks.cbegin(),
                                 devSymlinks.cend(),
                                 [](QString symlink) { return symlink.contains(kOverleyEncPrefix); });

    if (device.startsWith("/dev/dm-")
        && idType != "crypto_LUKS"
        && !isOverlay) {
        fmInfo() << "Mapper device is not supported for encryption:" << device << devSymlinks;
        return false;
    }

    const QStringList &supportedFS { "ext4", "ext3", "ext2" };
    param.states = static_cast<EncryptStates>(EventsHandler::instance()->deviceEncryptStatus(device));
    if ((param.states == EncryptState::kStatusNotEncrypted)
        && !supportedFS.contains(idType)) {
        fmInfo() << "Unsupported filesystem for encryption:" << idType;
        return false;
    } else if (idType == "crypto_LUKS"
               && selectedItemInfo.value("IdVersion").toString() == "1") {
        fmInfo() << "LUKS version 1 is not supported";
        return false;
    }

    QString devMpt = selectedItemInfo.value("MountPoint", "").toString();
    if (devMpt.isEmpty() && selectedItemInfo.contains("ClearBlockDeviceInfo"))
        devMpt = selectedItemInfo.value("ClearBlockDeviceInfo").toHash().value("MountPoint").toString();

    if (kDisabledEncryptPath.contains(devMpt, Qt::CaseInsensitive)) {
        fmInfo() << "Mount point does not support encryption:" << devMpt;
        return false;
    }

    auto mpts = selectedItemInfo.value("MountPoints").toStringList();
    if (mpts.contains("/") && QStorageInfo("/").device() == QStorageInfo("/boot").device()) {
        fmInfo() << "No separated Boot device, / cannot be encrypted.";
        return false;
    }

    param.jobType = job_type::TypeNormal;
    if (isOverlay) {
        param.jobType = job_type::TypeOverlay;
    } else {
        auto configJson = selectedItemInfo.value("Configuration", "").toString();
        if (selectedItemInfo.value("ClearBlockDeviceInfo").toHash().contains("Configuration")) {
            configJson = selectedItemInfo.value("ClearBlockDeviceInfo").toHash().value("Configuration").toString();
        }

        if (!configJson.isEmpty()) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(configJson.toLocal8Bit(), &err);
            if (err.error != QJsonParseError::NoError) {
                fmWarning() << "Device configuration not valid:" << device << configJson << "error:" << err.errorString();
                return false;
            }
            auto obj = doc.object();
            if (obj.contains("fstab"))
                param.jobType = job_type::TypeFstab;
            fmInfo() << device << "device configuration" << configJson << ", job type set to" << param.jobType;
        }
    }

    param.devID = selectedItemInfo.value("Id").toString();
    param.devDesc = device;
    param.mountPoint = devMpt;
    param.deviceDisplayName = info->displayOf(dfmbase::FileInfo::kFileDisplayName);
    param.secType = SecKeyType::kPwd;

    if (param.states != kStatusNotEncrypted) {
        param.secType = static_cast<SecKeyType>(device_utils::encKeyType(device));
        fmDebug() << "Device encryption finished, security type:" << param.secType;
    }

    param.devPhy = EventsHandler::instance()->holderDevice(device);

    return true;
}

bool DiskEncryptMenuScene::create(QMenu *)
{
    QAction *act = nullptr;

    act = new QAction(tr("Unlock encrypted partition"));
    act->setProperty(ActionPropertyKey::kActionID, kActIDUnlock);
    actions.insert(kActIDUnlock, act);

    act = new QAction(tr("Cancel partition encryption"));
    act->setProperty(ActionPropertyKey::kActionID, kActIDDecrypt);
    actions.insert(kActIDDecrypt, act);

    QString keyType = (param.secType == kPin) ? "PIN" : tr("passphrase");
    act = new QAction(tr("Changing the encryption %1").arg(keyType));
    act->setProperty(ActionPropertyKey::kActionID, kActIDChangePwd);
    actions.insert(kActIDChangePwd, act);

    act = new QAction(tr("Continue partition encryption"));
    act->setProperty(ActionPropertyKey::kActionID, kActIDResumeEncrypt);
    actions.insert(kActIDResumeEncrypt, act);

    act = new QAction(tr("Continue partition decryption"));
    act->setProperty(ActionPropertyKey::kActionID, kActIDResumeDecrypt);
    actions.insert(kActIDResumeDecrypt, act);

    act = new QAction(tr("Enable partition encryption"));
    act->setProperty(ActionPropertyKey::kActionID, kActIDEncrypt);
    actions.insert(kActIDEncrypt, act);

    return true;
}

bool DiskEncryptMenuScene::triggered(QAction *action)
{
    QString actID = action->property(ActionPropertyKey::kActionID).toString();

    if (actID == kActIDEncrypt) {
        fmInfo() << "Encrypt action triggered for device:" << param.devDesc;
        encryptDevice(param);
    } else if (actID == kActIDResumeEncrypt) {
        fmInfo() << "Resume encrypt action triggered for device:" << param.devDesc;
        EventsHandler::instance()->resumeEncrypt(param.devDesc);
    } else if (actID == kActIDDecrypt || actID == kActIDResumeDecrypt) {
        fmInfo() << "Decrypt/resume decrypt action triggered for device:" << param.devDesc;
        QString displayName = QString("%1(%2)").arg(param.deviceDisplayName).arg(param.devDesc.mid(5));

        if (dialog_utils::showConfirmDecryptionDialog(displayName, false) != QDialog::Accepted) {
            fmDebug() << "Decryption dialog cancelled by user";
            return true;
        }

        if (param.jobType == job_type::TypeNormal) {
            unmountBefore(decryptDevice, param);
        } else if (param.jobType == job_type::TypeOverlay) {
            // 在新的方案中，即便是需要重启的加密方案，在卷头初始化完成后，都会堆叠一层 dm 设备
            // 以便可以进行非重启的取消加密动作。复用 overlay 的方案。
            decryptDevice(param);
        } else {
            // 原 fstab 类型设备的取消加密动作，现应该走 overlay 的取消加密流程
            qWarning() << "something might be wrong, unsupported job type for decryption:"
                       << param.jobType
                       << param.devDesc;
            return false;
        }
    } else if (actID == kActIDChangePwd) {
        fmInfo() << "Change passphrase action triggered for device:" << param.devDesc;
        changePassphrase(param);
    } else if (actID == kActIDUnlock) {
        fmInfo() << "Unlock action triggered for device:" << selectedItemInfo.value("Id").toString();
        unlockDevice(selectedItemInfo.value("Id").toString());
    } else {
        fmWarning() << "Unknown action triggered:" << actID;
        return false;
    }
    return true;
}

void DiskEncryptMenuScene::updateState(QMenu *parent)
{
    sortActions(parent);
    updateActions();
}

void DiskEncryptMenuScene::encryptDevice(const DeviceEncryptParam &param)
{
    QString displayName = QString("%1(%2)").arg(param.deviceDisplayName).arg(param.devDesc.mid(5));

    // TypeFstab 始终需要重启
    // TypeOverlay 在禁用 useOverlayDMMode 配置时需要重启
    //   当以 fstab 方式加密完成后，设备会被堆叠一层 dm 设备，以便可以在运行时取消加密；取消加密之后，再次
    //   启动加密动作，设备会被识别为 overlay 类型，所以是否需要重启，在此时要考虑 useOverlayDMMode 配置项
    bool needreboot = (param.jobType == job_type::TypeFstab)
            || (param.jobType == job_type::TypeOverlay && !config_utils::useOverlayDMMode());

    int ret = dialog_utils::showConfirmEncryptionDialog(displayName, needreboot);
    if (ret != QDialog::Accepted) return;

    if (param.jobType == job_type::TypeNormal) {
        fmDebug() << "Normal job type, unmounting before encryption";
        unmountBefore(doEncryptDevice, param);
    } else {
        fmDebug() << "Special job type, proceeding directly to encryption";
        auto _param = param;
        _param.jobType = needreboot ? job_type::TypeFstab : job_type::TypeOverlay;
        doEncryptDevice(_param);
    }
}

void DiskEncryptMenuScene::decryptDevice(const DeviceEncryptParam &param)
{
    auto inputs = param;
    if (inputs.secType == kTpm) {
        fmInfo() << "TPM-based decryption, retrieving passphrase from TPM";
        QString passphrase = tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(inputs.devDesc, "");
        inputs.key = passphrase;
        if (passphrase.isEmpty()) {
            fmCritical() << "Failed to retrieve passphrase from TPM for device:" << inputs.devDesc;
            dialog_utils::showDialog(tr("Error"),
                                     tr("Cannot resolve passphrase from TPM"));
            UnlockPartitionDialog dlg(UnlockPartitionDialog::kRec);
            int ret = dlg.exec();
            if (ret != QDialog::Accepted)
                return;

            inputs.key = dlg.getUnlockKey().second;
        }
        doDecryptDevice(inputs);
        return;
    }

    DecryptParamsInputDialog dlg(inputs.devDesc);
    if (inputs.secType == kPin)
        dlg.setInputPIN(true);

    if (dlg.exec() != QDialog::Accepted) {
        fmDebug() << "Decrypt parameters dialog cancelled by user";
        return;
    }

    fmDebug() << "Starting decryption for device:" << inputs.devDesc;
    inputs.key = dlg.getKey();
    if (dlg.usingRecKey() || inputs.secType == kPwd) {
        fmDebug() << "Using recovery key or passphrase, proceeding to decrypt";
        doDecryptDevice(inputs);
    } else {
        fmDebug() << "Using PIN, retrieving TPM passphrase";
        inputs.key = tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(inputs.devDesc, inputs.key);
        if (inputs.key.isEmpty()) {
            fmCritical() << "PIN error: failed to retrieve TPM passphrase";
            dialog_utils::showDialog(tr("Error"), tr("PIN error"));
            return;
        }
        doDecryptDevice(inputs);
    }
}

void DiskEncryptMenuScene::changePassphrase(DeviceEncryptParam param)
{
    QString dev = param.devDesc;
    ChgPassphraseDialog dlg(param.devDesc);
    if (dlg.exec() != 1) {
        fmDebug() << "Change passphrase dialog cancelled by user";
        return;
    }

    auto inputs = dlg.getPassphrase();
    QString oldKey = inputs.first;
    QString newKey = inputs.second;
    if (param.secType == SecKeyType::kPin) {
        if (!dlg.validateByRecKey()) {
            fmDebug() << "Validating with PIN, retrieving TPM passphrase";
            oldKey = tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(dev, oldKey);
            if (oldKey.isEmpty()) {
                fmCritical() << "PIN error during passphrase change";
                dialog_utils::showDialog(tr("Error"), tr("PIN error"));
                return;
            }
        }
        QString newPassphrase;
        int ret = tpm_passphrase_utils::genPassphraseFromTPM_NonBlock(dev, newKey, &newPassphrase);
        if (ret != tpm_passphrase_utils::kTPMNoError) {
            fmCritical() << "Failed to generate new TPM passphrase, error:" << ret;
            dialog_utils::showTPMError(tr("Change passphrase failed"), static_cast<tpm_passphrase_utils::TPMError>(ret));
            return;
        }
        newKey = newPassphrase;
    }
    param.validateByRecKey = dlg.validateByRecKey();
    param.key = oldKey;
    param.newKey = newKey;
    doChangePassphrase(param);
}

void DiskEncryptMenuScene::unlockDevice(const QString &devObjPath)
{
    auto blkDev = device_utils::createBlockDevice(devObjPath);
    if (!blkDev) {
        fmWarning() << "Failed to create block device for:" << devObjPath;
        return;
    }

    QString pwd;
    bool cancel { false };
    bool ok = EventsHandler::instance()->onAcquireDevicePwd(blkDev->device(), &pwd, &cancel);
    if (pwd.isEmpty() && ok) {
        fmWarning() << "Failed to acquire password for device:" << blkDev->device();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    blkDev->unlockAsync(pwd, {}, onUnlocked);
}

void DiskEncryptMenuScene::doEncryptDevice(const DeviceEncryptParam &param)
{
    CREATE_DAEMON_INTERFACE(iface);
    if (!iface.isValid()) {
        fmCritical() << "Failed to create DBus interface for InitEncryption, service may be unavailable";
        return;
    }

    QVariantMap params {
        { encrypt_param_keys::kKeyDevice, param.devDesc },
        { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName },
        { encrypt_param_keys::kKeyMountPoint, param.mountPoint },
        { encrypt_param_keys::kKeyJobType, param.jobType },
        { encrypt_param_keys::kKeyPhyDevice, param.devPhy }
    };

    fmDebug() << "Calling InitEncryption D-Bus method";
    QDBusReply<bool> ret = iface.call("InitEncryption", params);
    if (ret.value()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    } else {
        fmCritical() << "Encryption initialization failed";
    }
}

void DiskEncryptMenuScene::doReencryptDevice(const DeviceEncryptParam &param)
{
    // if tpm selected, use tpm to generate the key
    QString tpmToken;
    if (param.secType != kPwd) {
        fmDebug() << "Generating TPM token for re-encryption";
        tpmToken = generateTPMToken(param.devDesc, param.secType == kPin);
    }

    CREATE_DAEMON_INTERFACE(iface);
    if (!iface.isValid()) {
        fmCritical() << "Failed to create DBus interface for SetupAuthArgs, service may be unavailable";
        return;
    }

    // Prepare credentials data
    QVariantMap params {
        { encrypt_param_keys::kKeyDevice, param.devDesc },
        { encrypt_param_keys::kKeyPassphrase, toBase64(param.key) },
        { encrypt_param_keys::kKeyExportToPath, param.exportPath },
    };
    if (!tpmToken.isEmpty())
        params.insert(encrypt_param_keys::kKeyTPMToken, tpmToken);

    // Send credentials via fd
    fmDebug() << "Starting device re-encryption via fd";
    if (sendCredentialsViaFd(iface, "SetupAuthArgs", params, true)) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }
}

void DiskEncryptMenuScene::doDecryptDevice(const DeviceEncryptParam &param)
{
    CREATE_DAEMON_INTERFACE(iface);
    if (!iface.isValid()) {
        fmCritical() << "Failed to create DBus interface for Decryption, service may be unavailable";
        return;
    }

    // Prepare credentials data
    QVariantMap params {
        { encrypt_param_keys::kKeyJobType, param.jobType },
        { encrypt_param_keys::kKeyDevice, param.devDesc },
        { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName },
        { encrypt_param_keys::kKeyPassphrase, toBase64(param.key) }
    };

    // Send credentials via fd
    fmDebug() << "Calling Decryption D-Bus method via fd";
    if (sendCredentialsViaFd(iface, "Decryption", params, false)) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        EventsHandler::instance()->setAutoStartDFM(true);
    } else {
        fmCritical() << "Decryption failed to start";
    }
}

void DiskEncryptMenuScene::doChangePassphrase(const DeviceEncryptParam &param)
{
    QString token;
    if (param.secType != SecKeyType::kPwd) {
        // new tpm token should be setted.
        QFile f(kGlobalTPMConfigPath + param.devDesc + "/token.json");
        if (!f.open(QIODevice::ReadOnly)) {
            fmCritical() << "Cannot read old TPM token for device:" << param.devDesc;
            return;
        }
        QJsonDocument oldTokenDoc = QJsonDocument::fromJson(f.readAll());
        f.close();
        QJsonObject oldTokenObj = oldTokenDoc.object();

        QString newToken = generateTPMToken(param.devDesc, param.secType == SecKeyType::kPin);
        QJsonDocument newTokenDoc = QJsonDocument::fromJson(newToken.toLocal8Bit());
        QJsonObject newTokenObj = newTokenDoc.object();

        oldTokenObj.insert("enc", newTokenObj.value("enc"));
        oldTokenObj.insert("kek-priv", newTokenObj.value("kek-priv"));
        oldTokenObj.insert("kek-pub", newTokenObj.value("kek-pub"));
        oldTokenObj.insert("iv", newTokenObj.value("iv"));
        newTokenDoc.setObject(oldTokenObj);
        token = newTokenDoc.toJson(QJsonDocument::Compact);
    }

    CREATE_DAEMON_INTERFACE(iface);
    if (!iface.isValid()) {
        fmCritical() << "Failed to create DBus interface for ChangePassphrase, service may be unavailable";
        return;
    }

    // Create anonymous pipe for secure credential transmission
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        fmCritical() << "Failed to create anonymous pipe for credentials";
        return;
    }

    // Prepare credentials data using QDataStream for reliable serialization
    QByteArray credentials;
    QDataStream stream(&credentials, QIODevice::WriteOnly);
    QVariantMap params {
        { encrypt_param_keys::kKeyDevice, param.devDesc },
        { encrypt_param_keys::kKeyPassphrase, toBase64(param.newKey) },
        { encrypt_param_keys::kKeyOldPassphrase, toBase64(param.key) },
        { encrypt_param_keys::kKeyValidateWithRecKey, param.validateByRecKey },
        { encrypt_param_keys::kKeyTPMToken, token },
        { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName }
    };
    stream << params;

    // Write credentials to pipe and close write end immediately
    ssize_t written = write(pipefd[1], credentials.constData(), credentials.size());
    close(pipefd[1]);   // Close write end immediately after writing

    if (written != credentials.size()) {
        fmCritical() << "Failed to write credentials to pipe, written:" << written << "expected:" << credentials.size();
        close(pipefd[0]);
        return;
    }

    // Create file descriptor for D-Bus transmission
    QDBusUnixFileDescriptor fd(pipefd[0]);
    if (!fd.isValid()) {
        fmCritical() << "Failed to create valid file descriptor from pipe";
        close(pipefd[0]);
        return;
    }

    fmDebug() << "Calling ChangePassphrase D-Bus method via fd";
    QDBusReply<bool> ret = iface.call("ChangePassphrase", QVariant::fromValue(fd));
    if (ret.value()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    } else {
        fmCritical() << "Passphrase change failed to start";
    }

    // Close read end (D-Bus service will have its own copy)
    close(pipefd[0]);
}

QString DiskEncryptMenuScene::generateTPMConfig()
{
    QString sessionHashAlgo, sessionKeyAlgo, primaryHashAlgo, primaryKeyAlgo, minorHashAlgo, minorKeyAlgo, pcr, pcrbank;
    if (!tpm_passphrase_utils::getAlgorithm(&sessionHashAlgo, &sessionKeyAlgo,
                                            &primaryHashAlgo, &primaryKeyAlgo,
                                            &minorHashAlgo, &minorKeyAlgo,
                                            &pcr, &pcrbank)) {
        fmCritical() << "Cannot choose algorithm for TPM";
        return "";
    }

    QJsonObject tpmParams;
    tpmParams = { { "keyslot", "1" },
                  { "session-hash-alg", sessionHashAlgo },
                  { "session-key-alg", sessionKeyAlgo },
                  { "primary-hash-alg", primaryHashAlgo },
                  { "primary-key-alg", primaryKeyAlgo },
                  { "minor-hash-alg", minorHashAlgo },
                  { "minor-key-alg", minorKeyAlgo },
                  { "pcr", pcr },
                  { "pcr-bank", pcrbank } };
    return QJsonDocument(tpmParams).toJson();
}

QString DiskEncryptMenuScene::generateTPMToken(const QString &device, bool pin)
{
    QString tpmConfig = generateTPMConfig();
    QJsonDocument doc = QJsonDocument::fromJson(tpmConfig.toLocal8Bit());
    QJsonObject token = doc.object();

    // keep same with usec.
    // j["type"] = "usec-tpm2";
    // j["keyslots"] = {"0"};
    // j["kek-priv"] = encoded_priv_key;
    // j["kek-pub"] = encoded_pub_key;
    // j["primary-key-alg"] = primary_key_alg;
    // j["primary-hash-alg"] = primary_hash_alg;
    // j["iv"] = encoded_iv;
    // j["enc"] = encoded_cipher;
    // j["pin"] = pin;
    // j["pcr"] = pcr;
    // j["pcr-bank"] = pcr_bank;

    token.remove("keyslot");
    token.insert("type", "usec-tpm2");
    token.insert("keyslots", QJsonArray::fromStringList({ "0" }));
    token.insert("kek-priv", getBase64Of(kGlobalTPMConfigPath + device + "/key.priv"));
    token.insert("kek-pub", getBase64Of(kGlobalTPMConfigPath + device + "/key.pub"));
    token.insert("iv", getBase64Of(kGlobalTPMConfigPath + device + "/iv.bin"));
    token.insert("enc", getBase64Of(kGlobalTPMConfigPath + device + "/cipher.out"));
    token.insert("pin", pin ? "1" : "0");

    doc.setObject(token);
    return doc.toJson(QJsonDocument::Compact);
}

QString DiskEncryptMenuScene::getBase64Of(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        fmWarning() << "Cannot read file:" << fileName;
        return "";
    }
    QByteArray contents = f.readAll();
    f.close();
    return QString(contents.toBase64());
}

bool DiskEncryptMenuScene::sendCredentialsViaFd(QDBusInterface &iface, const QString &method,
                                                const QVariantMap &params, bool asyncCall)
{
    // Create anonymous pipe for secure credential transmission
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        fmCritical() << "[sendCredentialsViaFd] Failed to create anonymous pipe for credentials";
        return false;
    }

    // Prepare credentials data using QDataStream for reliable serialization
    QByteArray credentials;
    QDataStream stream(&credentials, QIODevice::WriteOnly);
    stream << params;

    // Write credentials to pipe and close write end immediately
    ssize_t written = write(pipefd[1], credentials.constData(), credentials.size());
    close(pipefd[1]);   // Close write end immediately after writing

    if (written != credentials.size()) {
        fmCritical() << "[sendCredentialsViaFd] Failed to write credentials to pipe, written:" << written << "expected:" << credentials.size();
        close(pipefd[0]);
        return false;
    }

    // Create file descriptor for D-Bus transmission
    QDBusUnixFileDescriptor fd(pipefd[0]);
    if (!fd.isValid()) {
        fmCritical() << "[sendCredentialsViaFd] Failed to create valid file descriptor from pipe";
        close(pipefd[0]);
        return false;
    }

    // Call D-Bus method with file descriptor
    fmDebug() << "[sendCredentialsViaFd] Calling D-Bus method:" << method << "via fd";
    if (asyncCall) {
        iface.asyncCall(method, QVariant::fromValue(fd));
    } else {
        QDBusReply<bool> reply = iface.call(method, QVariant::fromValue(fd));
        close(pipefd[0]);
        return reply.value();
    }

    // Close read end (D-Bus service will have its own copy)
    close(pipefd[0]);
    return true;
}

void DiskEncryptMenuScene::onUnlocked(bool ok, dfmmount::OperationErrorInfo info, QString clearDev)
{
    QApplication::restoreOverrideCursor();
    if (!ok && info.code != dfmmount::DeviceError::kUDisksErrorNotAuthorizedDismissed) {
        fmWarning() << "Unlock device failed:" << info.message;
        dialog_utils::showDialog(tr("Unlock partition failed"),
                                 tr("Wrong passphrase"));
        return;
    }

    auto dev = device_utils::createBlockDevice(clearDev);
    if (!dev) {
        fmWarning() << "Failed to create block device for clear device:" << clearDev;
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    dev->mountAsync({}, onMounted);
}

void DiskEncryptMenuScene::onMounted(bool ok, dfmmount::OperationErrorInfo info, QString mountPoint)
{
    QApplication::restoreOverrideCursor();
    if (!ok && info.code != dfmmount::DeviceError::kUDisksErrorNotAuthorizedDismissed) {
        fmWarning() << "Mount device failed:" << info.message;
        dialog_utils::showDialog(tr("Mount device failed"), "");
        return;
    }
}

void DiskEncryptMenuScene::unmountBefore(const std::function<void(const DeviceEncryptParam &)> &after, const DeviceEncryptParam &param)
{
    using namespace dfmmount;
    auto blk = device_utils::createBlockDevice(param.devID);
    if (!blk) {
        fmWarning() << "Failed to create block device for unmount:" << param.devID;
        return;
    }

    auto _params = param;
    if (blk->isEncrypted()) {
        fmDebug() << "Device is encrypted, processing clear device";
        const QString &clearPath = blk->getProperty(Property::kEncryptedCleartextDevice).toString();
        if (clearPath.length() > 1) {
            fmDebug() << "Clear device found:" << clearPath;

            auto lock = [=] {
                blk->lockAsync({}, [=](bool ok, OperationErrorInfo err) {
                    ok ? after(_params) : onUnmountError(kLock, _params.devDesc, err);
                });
            };
            auto onUnmounted = [=](bool ok, const OperationErrorInfo &err) {
                ok ? lock() : onUnmountError(kUnmount, _params.devDesc, err);
            };

            // do unmount cleardev
            auto clearDev = device_utils::createBlockDevice(clearPath);
            clearDev->unmountAsync({}, onUnmounted);
        } else {
            fmDebug() << "No clear device found, proceeding with operation";
            after(_params);
        }
    } else {
        fmDebug() << "Device is not encrypted, unmounting directly";
        blk->unmountAsync({}, [=](bool ok, OperationErrorInfo err) {
            ok ? after(_params) : onUnmountError(kUnmount, _params.devDesc, err);
        });
    }
}

void DiskEncryptMenuScene::onUnmountError(OpType t, const QString &dev, const dfmmount::OperationErrorInfo &err)
{
    fmWarning() << "Unmount operation failed for device:" << dev << "operation type:" << t << "error:" << err.message;

    QString operation = (t == kUnmount) ? tr("unmount") : tr("lock");
    dialog_utils::showDialog(tr("Encrypt failed"),
                             tr("Cannot %1 device %2").arg(operation, dev));
}

void DiskEncryptMenuScene::sortActions(QMenu *parent)
{
    Q_ASSERT(parent);
    QList<QAction *> acts = parent->actions();
    if (acts.isEmpty()) {
        fmDebug() << "No actions to sort";
        return;
    }

    QAction *before { acts.last() };
    for (int i = 0; i < acts.count(); ++i) {
        auto act = acts.at(i);
        QString actID = act->property(ActionPropertyKey::kActionID).toString();
        if (actID == "computer-rename"   // the encrypt actions should be under computer-rename
            && (i + 1) < acts.count()) {
            before = acts.at(i + 1);
            break;
        }
    }

    std::for_each(actions.begin(), actions.end(), [=](QAction *act) {
        parent->insertAction(before, act);
        act->setParent(parent);
    });
}

void DiskEncryptMenuScene::updateActions()
{
    std::for_each(actions.begin(), actions.end(), [](QAction *act) {
        act->setVisible(false);
        act->setEnabled(false);
    });

    // update operatable
    bool taskWorking = EventsHandler::instance()->isTaskWorking();
    bool currDevOperating = EventsHandler::instance()->isUnderOperating(param.devDesc)
            || EventsHandler::instance()->isUnderOperating(param.devPhy);
    // 允许在存在加密配置的时候操作其他分区的加解密动作
    bool hasPendingJob = 0 && EventsHandler::instance()->hasPendingTask();
    actions[kActIDEncrypt]->setEnabled(!taskWorking && !hasPendingJob);
    actions[kActIDDecrypt]->setEnabled(!taskWorking && !hasPendingJob && !currDevOperating);
    actions[kActIDChangePwd]->setEnabled(!taskWorking);
    actions[kActIDResumeEncrypt]->setEnabled(!taskWorking && !currDevOperating);
    actions[kActIDResumeDecrypt]->setEnabled(!taskWorking && !currDevOperating);
    actions[kActIDUnlock]->setEnabled(!currDevOperating);

    // update visibility
    if (param.states & ~EncryptState::kStatusNotEncrypted) {
        // fully encrypted
        if (param.states & kStatusFinished) {
            bool unlocked = selectedItemInfo.value("CleartextDevice").toString() != "/";
            actions[kActIDDecrypt]->setVisible(true);
            actions[kActIDUnlock]->setVisible(!unlocked);
            if (param.secType != disk_encrypt::kTpm)
                actions[kActIDChangePwd]->setVisible(true);
        }
        // not finished
        else if (param.states & kStatusOnline) {
            // encrytp not finish
            if (param.states & kStatusEncrypt) {
                actions[kActIDResumeEncrypt]->setVisible(true);
            }
            // decrypt not finish
            else if (param.states & kStatusDecrypt) {
                actions[kActIDDecrypt]->setVisible(false);
                actions[kActIDResumeDecrypt]->setVisible(true);
            }
        } else {
            fmWarning() << "Unmet encryption status:" << param.devDesc << param.states;
        }
    } else if (EventsHandler::instance()->unfinishedDecryptJob() == param.devDesc
               || EventsHandler::instance()->unfinishedDecryptJob() == param.devPhy) {
        actions[kActIDResumeDecrypt]->setVisible(true);
    } else {
        actions[kActIDEncrypt]->setVisible(true);
    }

    // Check for reboot flags
    QString dev = param.devDesc;
    QString fileName = kRebootFlagFilePrefix + dev.replace("/", "_");
    QFile rebootFlag(fileName);
    if (rebootFlag.exists()) {
        actions[kActIDEncrypt]->setText(tr("Reboot to continue encrypt"));
        actions[kActIDDecrypt]->setText(tr("Reboot to finish decrypt"));
    }
}
