// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmplugin_disk_encrypt_global.h"
#include "diskencryptmenuscene.h"
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
    auto devSymlinks = selectedItemInfo.value("Symlinks").toStringList();
    bool isOverlay = std::any_of(devSymlinks.cbegin(),
                                 devSymlinks.cend(),
                                 [](QString symlink) { return symlink.contains(kOverleyEncPrefix); });
    if (device.startsWith("/dev/dm-")
        && idType != "crypto_LUKS"
        && !isOverlay) {
        qInfo() << "mapper device is not supported to be encrypted yet." << device << devSymlinks;
        return false;
    }

    const QStringList &supportedFS { "ext4", "ext3", "ext2" };
    param.states = static_cast<EncryptStates>(EventsHandler::instance()->deviceEncryptStatus(device));
    if ((param.states == EncryptState::kStatusNotEncrypted)
        && !supportedFS.contains(idType)) {
        return false;
    } else if (idType == "crypto_LUKS"
               && selectedItemInfo.value("IdVersion").toString() == "1") {
        return false;
    }

    QString devMpt = selectedItemInfo.value("MountPoint", "").toString();
    if (devMpt.isEmpty() && selectedItemInfo.contains("ClearBlockDeviceInfo"))
        devMpt = selectedItemInfo.value("ClearBlockDeviceInfo").toHash().value("MountPoint").toString();

    if (kDisabledEncryptPath.contains(devMpt, Qt::CaseInsensitive)) {
        qInfo() << devMpt << "doesn't support encrypt";
        return false;
    }

    param.jobType = job_type::TypeNormal;
    if (isOverlay) {
        param.jobType = job_type::TypeOverlay;
    } else {
        auto configJson = selectedItemInfo.value("Configuration", "").toString();
        if (!configJson.isEmpty()) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(configJson.toLocal8Bit(), &err);
            if (err.error != QJsonParseError::NoError) {
                qWarning() << "device configuration not valid!" << device << configJson;
                return false;
            }
            auto obj = doc.object();
            param.jobType = job_type::TypeFstab;
        }
    }

    param.devID = selectedItemInfo.value("Id").toString();
    param.devDesc = device;
    param.mountPoint = devMpt;
    param.deviceDisplayName = info->displayOf(dfmbase::FileInfo::kFileDisplayName);
    param.secType = SecKeyType::kPwd;

    if (param.states & EncryptState::kStatusFinished)
        param.secType = static_cast<SecKeyType>(device_utils::encKeyType(device));

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
        encryptDevice(param);
    } else if (actID == kActIDResumeEncrypt) {
        EventsHandler::instance()->resumeEncrypt(param.devDesc);
    } else if (actID == kActIDDecrypt || actID == kActIDResumeDecrypt) {
        QString displayName = QString("%1(%2)").arg(param.deviceDisplayName).arg(param.devDesc.mid(5));
        if (dialog_utils::showConfirmDecryptionDialog(displayName, param.jobType == job_type::TypeFstab) != QDialog::Accepted)
            return true;
        if (param.jobType == job_type::TypeNormal)
            unmountBefore(decryptDevice, param);
        else if (param.jobType == job_type::TypeOverlay)
            decryptDevice(param);
        else if (param.jobType == job_type::TypeFstab)
            doDecryptDevice(param);
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
    bool needreboot = param.jobType == job_type::TypeFstab;
    int ret = dialog_utils::showConfirmEncryptionDialog(displayName, needreboot);
    if (ret != QDialog::Accepted) return;

    if (param.jobType == job_type::TypeNormal)
        unmountBefore(doEncryptDevice, param);
    else
        doEncryptDevice(param);
}

void DiskEncryptMenuScene::decryptDevice(const DeviceEncryptParam &param)
{
    auto inputs = param;
    if (inputs.secType == kTpm) {
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
    if (inputs.secType == kPin)
        dlg.setInputPIN(true);

    if (dlg.exec() != QDialog::Accepted)
        return;

    qDebug() << "start decrypting device" << inputs.devDesc;
    inputs.key = dlg.getKey();
    if (dlg.usingRecKey() || inputs.secType == kPwd)
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
    if (param.secType == SecKeyType::kPin) {
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
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QVariantMap params {
            { encrypt_param_keys::kKeyDevice, param.devDesc },
            { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName },
            { encrypt_param_keys::kKeyMountPoint, param.mountPoint },
            { encrypt_param_keys::kKeyJobType, param.jobType }
        };

        QDBusReply<bool> ret = iface.call("InitEncryption", params);
        if (ret.value())
            QApplication::setOverrideCursor(Qt::WaitCursor);
    }
}

void DiskEncryptMenuScene::doReencryptDevice(const DeviceEncryptParam &param)
{
    // if tpm selected, use tpm to generate the key
    QString tpmToken;
    if (param.secType != kPwd)
        tpmToken = generateTPMToken(param.devDesc, param.secType == kPin);

    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QVariantMap params {
            { encrypt_param_keys::kKeyDevice, param.devDesc },
            { encrypt_param_keys::kKeyPassphrase, toBase64(param.key) },
            { encrypt_param_keys::kKeyExportToPath, param.exportPath },
        };
        if (!tpmToken.isEmpty()) params.insert(encrypt_param_keys::kKeyTPMToken, tpmToken);

        qDebug() << "start reencrypt device";
        QDBusReply<bool> ret = iface.call("SetupAuthArgs", params);
        if (ret.value())
            QApplication::setOverrideCursor(Qt::WaitCursor);

    }
}

void DiskEncryptMenuScene::doDecryptDevice(const DeviceEncryptParam &param)
{
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QVariantMap params {
            { encrypt_param_keys::kKeyJobType, param.jobType },
            { encrypt_param_keys::kKeyDevice, param.devDesc },
            { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName },
            { encrypt_param_keys::kKeyPassphrase, toBase64(param.key) }
        };
        QDBusReply<bool> ret = iface.call("Decryption", params);
        if (ret.value())
            QApplication::setOverrideCursor(Qt::WaitCursor);

        EventsHandler::instance()->autoStartDFM();
    }
}

void DiskEncryptMenuScene::doChangePassphrase(const DeviceEncryptParam &param)
{
    QString token;
    if (param.secType != SecKeyType::kPwd) {
        // new tpm token should be setted.
        QFile f(kGlobalTPMConfigPath + param.devDesc + "/token.json");
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning() << "cannot read old tpm token!!!";
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

    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QVariantMap params {
            { encrypt_param_keys::kKeyDevice, param.devDesc },
            { encrypt_param_keys::kKeyPassphrase, toBase64(param.newKey) },
            { encrypt_param_keys::kKeyOldPassphrase, toBase64(param.key) },
            { encrypt_param_keys::kKeyValidateWithRecKey, param.validateByRecKey },
            { encrypt_param_keys::kKeyTPMToken, token },
            { encrypt_param_keys::kKeyDeviceName, param.deviceDisplayName }
        };
        QDBusReply<bool> ret = iface.call("ChangePassphrase", params);
        if (ret.value())
            QApplication::setOverrideCursor(Qt::WaitCursor);
    }
}

QString DiskEncryptMenuScene::generateTPMConfig()
{
    QString sessionHashAlgo, sessionKeyAlgo, primaryHashAlgo, primaryKeyAlgo, minorHashAlgo, minorKeyAlgo, pcr, pcrbank;
    if (!tpm_passphrase_utils::getAlgorithm(&sessionHashAlgo, &sessionKeyAlgo,
                                            &primaryHashAlgo, &primaryKeyAlgo,
                                            &minorHashAlgo, &minorKeyAlgo,
                                            &pcr, &pcrbank)) {
        qWarning() << "cannot choose algorithm for tpm";
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
    if (acts.isEmpty())
        return;

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
    bool hasPendingJob = EventsHandler::instance()->hasPendingTask();
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
            qWarning() << "unmet status!" << param.devDesc << param.states;
        }
    } else if (EventsHandler::instance()->unfinishedDecryptJob() == param.devDesc
               || EventsHandler::instance()->unfinishedDecryptJob() == param.devPhy) {
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
