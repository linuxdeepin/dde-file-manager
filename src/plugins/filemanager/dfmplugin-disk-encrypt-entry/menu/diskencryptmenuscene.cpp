// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmplugin_disk_encrypt_global.h"
#include "diskencryptmenuscene.h"
#include "gui/encryptparamsinputdialog.h"
#include "gui/decryptparamsinputdialog.h"
#include "gui/chgpassphrasedialog.h"
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

#include <ddialog.h>
#include <dconfig.h>

#include <fstab.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_diskenc;
using namespace disk_encrypt;

static constexpr char kActIDEncrypt[] { "de_0_encrypt" };
static constexpr char kActIDResumeEncrypt[] { "de_0_resumeEncrypt" };
static constexpr char kActIDUnlock[] { "de_0_unlock" };
static constexpr char kActIDDecrypt[] { "de_1_decrypt" };
static constexpr char kActIDResumeDecrypt[] { "de_1_resumeDecrypt" };
static constexpr char kActIDChangePwd[] { "de_2_changePwd" };

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
        qInfo() << "partition encryption feature is disabled.";
        return false;
    }

    QList<QUrl> selectedItems = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (selectedItems.isEmpty())
        return false;

    auto selectedItem = selectedItems.first();
    if (!selectedItem.path().endsWith("blockdev"))
        return false;

    QSharedPointer<FileInfo> info = InfoFactory::create<FileInfo>(selectedItem);
    if (!info)
        return false;
    info->refresh();

    selectedItemInfo = info->extraProperties();
    auto device = selectedItemInfo.value("Device", "").toString();
    if (device.isEmpty())
        return false;

    const QString &idType = selectedItemInfo.value("IdType").toString();
    auto preferDev = selectedItemInfo.value("PreferredDevice", "").toString();
    if (device.startsWith("/dev/dm-") && idType != "crypto_LUKS") {
        qInfo() << "mapper device is not supported to be encrypted yet." << device << preferDev;
        return false;
    }

    const QStringList &supportedFS { "ext4", "ext3", "ext2" };
    if (idType == "crypto_LUKS") {
        if (selectedItemInfo.value("IdVersion").toString() == "1")
            return false;
        hasCryptHeader = true;
    } else if (!supportedFS.contains(idType)) {
        return false;
    }

    QString devMpt = selectedItemInfo.value("MountPoint", "").toString();
    if (devMpt.isEmpty() && selectedItemInfo.contains("ClearBlockDeviceInfo"))
        devMpt = selectedItemInfo.value("ClearBlockDeviceInfo").toHash().value("MountPoint").toString();

    if (kDisabledEncryptPath.contains(devMpt, Qt::CaseInsensitive)) {
        qInfo() << devMpt << "doesn't support encrypt";
        return false;
    }

    param.initOnly = false;
    auto configJson = selectedItemInfo.value("Configuration", "").toString();
    if (!configJson.isEmpty()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(configJson.toLocal8Bit(), &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "device configuration not valid!" << device << configJson;
            return false;
        }
        auto obj = doc.object();
        param.initOnly = obj.contains("fstab");
    }
    param.devID = selectedItemInfo.value("Id").toString();
    param.devDesc = device;
    param.mountPoint = devMpt;
    param.uuid = selectedItemInfo.value("IdUUID", "").toString();
    param.deviceDisplayName = info->displayOf(dfmbase::FileInfo::kFileDisplayName);
    param.type = SecKeyType::kPasswordOnly;
    param.backingDevUUID = param.uuid;
    param.isDetachedHeader = false;

    QVariantHash clearInfo = selectedItemInfo.value("ClearBlockDeviceInfo").toHash();
    if (!clearInfo.isEmpty()) {
        param.clearDevUUID = clearInfo.value("IdUUID", "").toString();
        param.prefferDevName = clearInfo.value("PreferredDevice").toString().mid(12);   // /dev/mapper/xxxx ==> xxxx
    }

    if (hasCryptHeader)
        param.type = static_cast<SecKeyType>(device_utils::encKeyType(device));

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

    QString keyType = (param.type == kTPMAndPIN) ? "PIN" : tr("passphrase");
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
        encryptDevice(param);
    } else if (actID == kActIDResumeEncrypt) {
        EventsHandler::instance()->resumeEncrypt(param.devDesc);
    } else if (actID == kActIDDecrypt || actID == kActIDResumeDecrypt) {
        QString displayName = QString("%1(%2)").arg(param.deviceDisplayName).arg(param.devDesc.mid(5));
        if (dialog_utils::showConfirmDecryptionDialog(displayName, param.initOnly) != QDialog::Accepted)
            return true;
        param.initOnly ? doDecryptDevice(param) : unmountBefore(decryptDevice, param);
    } else if (actID == kActIDChangePwd) {
        changePassphrase(param);
    } else if (actID == kActIDUnlock) {
        unlockDevice(selectedItemInfo.value("Id").toString());
    } else {
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
    bool needreboot = param.isDetachedHeader ? false : param.initOnly;
    int ret = dialog_utils::showConfirmEncryptionDialog(displayName, needreboot);
    if (ret == QDialog::Accepted) {
        if (param.initOnly)
            doEncryptDevice(param);
        else
            unmountBefore(doEncryptDevice, param);
    }
}

void DiskEncryptMenuScene::decryptDevice(const DeviceEncryptParam &param)
{
    auto inputs = param;
    if (inputs.type == kTPMOnly) {
        QString passphrase = tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(inputs.devDesc, "");
        inputs.key = passphrase;
        if (passphrase.isEmpty()) {
            dialog_utils::showDialog(tr("Error"),
                                     tr("Cannot resolve passphrase from TPM"),
                                     dialog_utils::DialogType::kError);
            return;
        }
        doDecryptDevice(inputs);
        return;
    }

    DecryptParamsInputDialog dlg(inputs.devDesc);
    if (inputs.type == kTPMAndPIN)
        dlg.setInputPIN(true);

    if (dlg.exec() != QDialog::Accepted)
        return;

    qDebug() << "start decrypting device" << inputs.devDesc;
    inputs.key = dlg.getKey();
    if (dlg.usingRecKey() || inputs.type == kPasswordOnly)
        doDecryptDevice(inputs);
    else {
        inputs.key = tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(inputs.devDesc, inputs.key);
        if (inputs.key.isEmpty()) {
            dialog_utils::showDialog(tr("Error"), tr("PIN error"), dialog_utils::DialogType::kError);
            return;
        }
        doDecryptDevice(inputs);
    }
}

void DiskEncryptMenuScene::changePassphrase(DeviceEncryptParam param)
{
    QString dev = param.devDesc;
    ChgPassphraseDialog dlg(param.devDesc);
    if (dlg.exec() != 1)
        return;

    auto inputs = dlg.getPassphrase();
    QString oldKey = inputs.first;
    QString newKey = inputs.second;
    if (param.type == SecKeyType::kTPMAndPIN) {
        if (!dlg.validateByRecKey()) {
            oldKey = tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(dev, oldKey);
            if (oldKey.isEmpty()) {
                dialog_utils::showDialog(tr("Error"), tr("PIN error"), dialog_utils::DialogType::kError);
                return;
            }
        }
        QString newPassphrase;
        int ret = tpm_passphrase_utils::genPassphraseFromTPM_NonBlock(dev, newKey, &newPassphrase);
        if (ret != tpm_passphrase_utils::kTPMNoError) {
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
    if (!blkDev)
        return;

    QString pwd;
    bool cancel { false };
    bool ok = EventsHandler::instance()->onAcquireDevicePwd(blkDev->device(), &pwd, &cancel);
    if (pwd.isEmpty() && ok) {
        qWarning() << "acquire pwd faield!!!";
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    blkDev->unlockAsync(pwd, {}, onUnlocked);
}

void DiskEncryptMenuScene::doEncryptDevice(const DeviceEncryptParam &param)
{
    // if tpm selected, use tpm to generate the key
    QString tpmConfig, tpmToken;
    if (param.type != kPasswordOnly) {
        tpmConfig = generateTPMConfig();
        tpmToken = generateTPMToken(param.devDesc, param.type == kTPMAndPIN);
    }

    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        auto blkDev = device_utils::createBlockDevice(param.devID);
        if (!blkDev) {
            qCritical() << "Create block device failed, the object is: " << param.devID;
            return;
        }
        QString partUuid { blkDev->getProperty(dfmmount::Property::kPartitionUUID).toString() };
        QVariantMap params {
            { encrypt_param_keys::kKeyDevice, param.devDesc },
            { encrypt_param_keys::kKeyUUID, param.uuid },
            { encrypt_param_keys::kKeyCipher, config_utils::cipherType() },
            { encrypt_param_keys::kKeyPassphrase, encryptPasswd(param.key) },
            { encrypt_param_keys::kKeyInitParamsOnly, param.isDetachedHeader ? false : param.initOnly },
            { encrypt_param_keys::kKeyRecoveryExportPath, param.exportPath },
            { encrypt_param_keys::kKeyEncMode, static_cast<int>(param.type) },
            { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName },
            { encrypt_param_keys::kKeyMountPoint, param.mountPoint },
            { encrypt_param_keys::kKeyIsDetachedHeader, param.isDetachedHeader },
            { encrypt_param_keys::kKeyPrefferDevice, param.prefferDevName },
            { encrypt_param_keys::kKeyClearDevUUID, param.clearDevUUID },
            { encrypt_param_keys::kKeyPartUUID, partUuid }
        };
        if (!tpmConfig.isEmpty()) params.insert(encrypt_param_keys::kKeyTPMConfig, tpmConfig);
        if (!tpmToken.isEmpty()) params.insert(encrypt_param_keys::kKeyTPMToken, tpmToken);

        QDBusReply<QString> reply = iface.call("PrepareEncryptDisk", params);
        qDebug() << "preencrypt device jobid:" << reply.value();
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }
}

void DiskEncryptMenuScene::doReencryptDevice(const DeviceEncryptParam &param)
{
    // if tpm selected, use tpm to generate the key
    QString tpmConfig, tpmToken;
    if (param.type != kPasswordOnly) {
        tpmConfig = generateTPMConfig();
        tpmToken = generateTPMToken(param.devDesc, param.type == kTPMAndPIN);
    }

    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QVariantMap params {
            { encrypt_param_keys::kKeyDevice, param.devDesc },
            { encrypt_param_keys::kKeyUUID, param.uuid },
            { encrypt_param_keys::kKeyCipher, config_utils::cipherType() },
            { encrypt_param_keys::kKeyPassphrase, encryptPasswd(param.key) },
            { encrypt_param_keys::kKeyInitParamsOnly, param.initOnly },
            { encrypt_param_keys::kKeyRecoveryExportPath, param.exportPath },
            { encrypt_param_keys::kKeyEncMode, static_cast<int>(param.type) },
            { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName },
            { encrypt_param_keys::kKeyMountPoint, param.mountPoint },
            { encrypt_param_keys::kKeyBackingDevUUID, param.backingDevUUID },
            { encrypt_param_keys::kKeyClearDevUUID, param.clearDevUUID }
        };
        if (!tpmConfig.isEmpty()) params.insert(encrypt_param_keys::kKeyTPMConfig, tpmConfig);
        if (!tpmToken.isEmpty()) params.insert(encrypt_param_keys::kKeyTPMToken, tpmToken);

        QDBusReply<void> reply = iface.call("SetEncryptParams", params);
        qDebug() << "start reencrypt device";
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }
}

void DiskEncryptMenuScene::doDecryptDevice(const DeviceEncryptParam &param)
{
    // if tpm selected, use tpm to generate the key
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QVariantMap params {
            { encrypt_param_keys::kKeyDevice, param.devDesc },
            { encrypt_param_keys::kKeyPassphrase, encryptPasswd(param.key) },
            { encrypt_param_keys::kKeyInitParamsOnly, param.initOnly },
            { encrypt_param_keys::kKeyUUID, param.uuid },
            { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName },
            { encrypt_param_keys::kKeyPrefferDevice, param.prefferDevName },
            { encrypt_param_keys::kKeyClearDevUUID, param.clearDevUUID }
        };
        QDBusReply<QString> reply = iface.call("DecryptDisk", params);
        qDebug() << "preencrypt device jobid:" << reply.value();
        QApplication::setOverrideCursor(Qt::WaitCursor);

        EventsHandler::instance()->autoStartDFM();
    }
}

void DiskEncryptMenuScene::doChangePassphrase(const DeviceEncryptParam &param)
{
    QString token;
    if (param.type != SecKeyType::kPasswordOnly) {
        // new tpm token should be setted.
        QFile f(kGlobalTPMConfigPath + param.devDesc + "/token.json");
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning() << "cannot read old tpm token!!!";
            return;
        }
        QJsonDocument oldTokenDoc = QJsonDocument::fromJson(f.readAll());
        f.close();
        QJsonObject oldTokenObj = oldTokenDoc.object();

        QString newToken = generateTPMToken(param.devDesc, param.type == SecKeyType::kTPMAndPIN);
        QJsonDocument newTokenDoc = QJsonDocument::fromJson(newToken.toLocal8Bit());
        QJsonObject newTokenObj = newTokenDoc.object();

        oldTokenObj.insert("enc", newTokenObj.value("enc"));
        oldTokenObj.insert("kek-priv", newTokenObj.value("kek-priv"));
        oldTokenObj.insert("kek-pub", newTokenObj.value("kek-pub"));
        oldTokenObj.insert("iv", newTokenObj.value("iv"));
        newTokenDoc.setObject(oldTokenObj);
        token = newTokenDoc.toJson(QJsonDocument::Compact);
    }

    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QVariantMap params {
            { encrypt_param_keys::kKeyDevice, param.devDesc },
            { encrypt_param_keys::kKeyPassphrase, encryptPasswd(param.newKey) },
            { encrypt_param_keys::kKeyOldPassphrase, encryptPasswd(param.key) },
            { encrypt_param_keys::kKeyValidateWithRecKey, param.validateByRecKey },
            { encrypt_param_keys::kKeyTPMToken, token },
            { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName }
        };
        QDBusReply<QString> reply = iface.call("ChangeEncryptPassphress", params);
        qDebug() << "modify device passphrase jobid:" << reply.value();
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }
}

QString DiskEncryptMenuScene::generateTPMConfig()
{
    QString sessionHashAlgo, sessionKeyAlgo, primaryHashAlgo, primaryKeyAlgo, minorHashAlgo, minorKeyAlgo;
    if (!tpm_passphrase_utils::getAlgorithm(&sessionHashAlgo, &sessionKeyAlgo, &primaryHashAlgo, &primaryKeyAlgo, &minorHashAlgo, &minorKeyAlgo)) {
        qWarning() << "cannot choose algorithm for tpm";
        primaryHashAlgo = "sha256";
        primaryKeyAlgo = "ecc";
    }

    QJsonObject tpmParams;
    tpmParams = { { "keyslot", "1" },
                  { "session-key-alg", sessionKeyAlgo },
                  { "session-hash-alg", sessionHashAlgo },
                  { "primary-key-alg", primaryKeyAlgo },
                  { "primary-hash-alg", primaryHashAlgo },
                  { "pcr", "7" },
                  { "pcr-bank", primaryHashAlgo } };
    return QJsonDocument(tpmParams).toJson();
}

QString DiskEncryptMenuScene::generateTPMToken(const QString &device, bool pin)
{
    QString tpmConfig = generateTPMConfig();
    QJsonDocument doc = QJsonDocument::fromJson(tpmConfig.toLocal8Bit());
    QJsonObject token = doc.object();

    // keep same with usec.
    // https://gerrit.uniontech.com/plugins/gitiles/usec-crypt-kit/+/refs/heads/master/src/boot-crypt/util.cpp
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
        qDebug() << "cannot read file of" << fileName;
        return "";
    }
    QByteArray contents = f.readAll();
    f.close();
    return QString(contents.toBase64());
}

void DiskEncryptMenuScene::onUnlocked(bool ok, dfmmount::OperationErrorInfo info, QString clearDev)
{
    QApplication::restoreOverrideCursor();
    if (!ok && info.code != dfmmount::DeviceError::kUDisksErrorNotAuthorizedDismissed) {
        qWarning() << "unlock device failed!" << info.message;
        dialog_utils::showDialog(tr("Unlock device failed"),
                                 tr("Wrong passphrase"),
                                 dialog_utils::kError);
        return;
    }

    auto dev = device_utils::createBlockDevice(clearDev);
    if (!dev)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    dev->mountAsync({}, onMounted);
}

void DiskEncryptMenuScene::onMounted(bool ok, dfmmount::OperationErrorInfo info, QString mountPoint)
{
    QApplication::restoreOverrideCursor();
    if (!ok && info.code != dfmmount::DeviceError::kUDisksErrorNotAuthorizedDismissed) {
        qWarning() << "mount device failed!" << info.message;
        dialog_utils::showDialog(tr("Mount device failed"), "", dialog_utils::kError);
        return;
    }
}

void DiskEncryptMenuScene::unmountBefore(const std::function<void(const DeviceEncryptParam &)> &after, const DeviceEncryptParam &param)
{
    using namespace dfmmount;
    auto blk = device_utils::createBlockDevice(param.devID);
    if (!blk)
        return;

    auto _params = param;
    if (blk->isEncrypted()) {
        const QString &clearPath = blk->getProperty(Property::kEncryptedCleartextDevice).toString();
        if (clearPath.length() > 1) {
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
            after(_params);
        }
    } else {
        blk->unmountAsync({}, [=](bool ok, OperationErrorInfo err) {
            ok ? after(_params) : onUnmountError(kUnmount, _params.devDesc, err);
        });
    }
}

void DiskEncryptMenuScene::onUnmountError(OpType t, const QString &dev, const dfmmount::OperationErrorInfo &err)
{
    qDebug() << "unmount device failed:"
             << dev
             << err.message;
    QString operation = (t == kUnmount) ? tr("unmount") : tr("lock");
    dialog_utils::showDialog(tr("Encrypt failed"),
                             tr("Cannot %1 device %2").arg(operation, dev),
                             dialog_utils::kError);
}

void DiskEncryptMenuScene::sortActions(QMenu *parent)
{
    Q_ASSERT(parent);
    QList<QAction *> acts = parent->actions();
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
    bool currDevOperating = EventsHandler::instance()->isUnderOperating(param.devDesc);
    bool hasPendingJob = EventsHandler::instance()->hasPendingTask();
    actions[kActIDEncrypt]->setEnabled(!taskWorking && !hasPendingJob);
    actions[kActIDDecrypt]->setEnabled(!taskWorking && !hasPendingJob && !currDevOperating);
    actions[kActIDChangePwd]->setEnabled(!taskWorking);
    actions[kActIDResumeEncrypt]->setEnabled(!taskWorking && !currDevOperating);
    actions[kActIDResumeDecrypt]->setEnabled(!taskWorking && !currDevOperating);
    actions[kActIDUnlock]->setEnabled(!currDevOperating);

    // update visibility
    if (hasCryptHeader) {
        int states = EventsHandler::instance()->deviceEncryptStatus(param.devDesc);
        // fully encrypted
        if (states & kStatusFinished) {
            bool unlocked = selectedItemInfo.value("CleartextDevice").toString() != "/";
            actions[kActIDDecrypt]->setVisible(true);
            actions[kActIDUnlock]->setVisible(!unlocked);
            if (param.type != disk_encrypt::kTPMOnly)
                actions[kActIDChangePwd]->setVisible(true);
        }
        // not finished
        else if (states & kStatusOnline) {
            // encrytp not finish
            if (states & kStatusEncrypt) {
                if (states & kStatusNoEncryptConfig) {
                    param.isDetachedHeader = true;
                    actions[kActIDEncrypt]->setVisible(true);
                } else {
                    param.isDetachedHeader = false;
                    actions[kActIDResumeEncrypt]->setVisible(true);
                }
            }
            // decrypt not finish
            else if (states & kStatusDecrypt) {
                actions[kActIDDecrypt]->setVisible(false);
                actions[kActIDResumeDecrypt]->setVisible(true);
            }
        } else {
            qWarning() << "unmet status!" << param.devDesc << states;
        }
    } else if (EventsHandler::instance()->unfinishedDecryptJob() == param.devDesc) {
        actions[kActIDResumeDecrypt]->setVisible(true);
    } else {
        actions[kActIDEncrypt]->setVisible(true);
    }

    QString dev = param.devDesc;
    QString fileName = kRebootFlagFilePrefix + dev.replace("/", "_");
    QFile rebootFlag(fileName);
    if (rebootFlag.exists()) {
        actions[kActIDEncrypt]->setText(tr("Reboot to continue encrypt"));
        actions[kActIDDecrypt]->setText(tr("Reboot to finish decrypt"));
    }
}

QString DiskEncryptMenuScene::encryptPasswd(const QString &passwd)
{
    QByteArray byteArray = passwd.toUtf8();
    QByteArray encodedByteArray = byteArray.toBase64();
    return QString::fromUtf8(encodedByteArray);
}
