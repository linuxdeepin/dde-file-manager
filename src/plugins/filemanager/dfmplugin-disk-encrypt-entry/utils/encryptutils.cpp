// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <dfm-framework/event/event.h>

#include <dfm-mount/dmount.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QSettings>
#include <QDBusInterface>
#include <QDBusReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QApplication>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <dconfig.h>
#include <DDialog>

#include <fstab.h>

Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QString *)

using namespace dfmplugin_diskenc;

bool config_utils::exportKeyEnabled()
{
    auto cfg = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                          "org.deepin.dde.file-manager.diskencrypt");
    cfg->deleteLater();
    return cfg->value("allowExportEncKey", true).toBool();
}

QString config_utils::cipherType()
{
    auto cfg = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                          "org.deepin.dde.file-manager.diskencrypt");
    cfg->deleteLater();
    auto cipher = cfg->value("encryptAlgorithm", "sm4").toString();
    QStringList supportedCipher { "sm4", "aes" };
    if (!supportedCipher.contains(cipher))
        return "sm4";
    return cipher;
}

int tpm_utils::checkTPM()
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_TPMIsAvailablePro").toInt();
}

int tpm_utils::checkTPMLockoutStatus()
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_CheckTPMLockoutPro").toInt();
}

int tpm_utils::getRandomByTPM(int size, QString *output)
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_GetRandomByTPMPro", size, output).toInt();
}

int tpm_utils::isSupportAlgoByTPM(const QString &algoName, bool *support)
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_IsTPMSupportAlgoPro", algoName, support).toInt();
}

int tpm_utils::encryptByTPM(const QVariantMap &map)
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_EncryptByTPMPro", map).toInt();
}

int tpm_utils::decryptByTPM(const QVariantMap &map, QString *psw)
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_DecryptByTPMPro", map, psw).toInt();
}

int tpm_utils::ownerAuthStatus()
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_OwnerAuthStatus").toInt();
}

int device_utils::encKeyType(const QString &dev)
{
    QDBusInterface iface(kDaemonBusName,
                         kDaemonBusPath,
                         kDaemonBusIface,
                         QDBusConnection::systemBus());
    if (iface.isValid()) {
        QDBusReply<QString> reply = iface.call("TpmToken", dev);
        if (!reply.isValid()) return 0;
        QString tokenJson = reply.value();
        if (tokenJson.isEmpty()) return 0;

        QJsonDocument doc = QJsonDocument::fromJson(tokenJson.toLocal8Bit());
        QJsonObject obj = doc.object();
        cacheToken(dev, obj.toVariantMap());
        QString usePin = obj.value("pin").toString("");
        if (usePin.isEmpty()) return 0;
        if (usePin == "1") return 1;
        if (usePin == "0") return 2;
    }
    return 0;
}

int tpm_passphrase_utils::genPassphraseFromTPM(const QString &dev, const QString &pin, QString *passphrase)
{
    Q_ASSERT(passphrase);

    if ((tpm_utils::getRandomByTPM(kPasswordSize, passphrase) != 0)
        || passphrase->isEmpty()) {
        qCritical() << "TPM get random number failed!";
        return kTPMNoRandomNumber;
    }

    const QString dirPath = kGlobalTPMConfigPath + dev;
    QDir dir(dirPath);
    if (!dir.exists())
        dir.mkpath(dirPath);

    QString sessionHashAlgo, sessionKeyAlgo, primaryHashAlgo, primaryKeyAlgo, minorHashAlgo, minorKeyAlgo, pcr, pcrbank;
    if (!getAlgorithm(&sessionHashAlgo, &sessionKeyAlgo,
                      &primaryHashAlgo, &primaryKeyAlgo,
                      &minorHashAlgo, &minorKeyAlgo,
                      &pcr, &pcrbank)) {
        qCritical() << "TPM algo choice failed!";
        return kTPMMissingAlog;
    }

    QVariantMap map {
        { "PropertyKey_SessionHashAlgo", sessionHashAlgo },
        { "PropertyKey_SessionKeyAlgo", sessionKeyAlgo },
        { "PropertyKey_PrimaryHashAlgo", primaryHashAlgo },
        { "PropertyKey_PrimaryKeyAlgo", primaryKeyAlgo },
        { "PropertyKey_MinorHashAlgo", minorHashAlgo },
        { "PropertyKey_MinorKeyAlgo", minorKeyAlgo },
        { "PropertyKey_Pcr", pcr },
        { "PropertyKey_PcrBank", pcrbank },
        { "PropertyKey_DirPath", dirPath },
        { "PropertyKey_Plain", *passphrase },
    };
    if (pin.isEmpty()) {
        map.insert("PropertyKey_EncryptType", kUseTpmAndPcr);
    } else {
        map.insert("PropertyKey_EncryptType", kUseTpmAndPcrAndPin);
        map.insert("PropertyKey_PinCode", pin);
    }

    int err = tpm_utils::encryptByTPM(map);
    if (err != 0) {
        qCritical() << "save to TPM failed!!!";
        return TPMError(err);
    }

    return kTPMNoError;
}

QString tpm_passphrase_utils::getPassphraseFromTPM(const QString &dev, const QString &pin)
{
    const QString dirPath = kGlobalTPMConfigPath + dev;
    QString tokenDocPath = dirPath + QDir::separator() + "token.json";
    QFile file(tokenDocPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open token.json!";
        return "";
    }
    QJsonDocument tokenDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject obj = tokenDoc.object();
    if (!obj.contains("session-hash-alg") || !obj.contains("session-key-alg")
        || !obj.contains("primary-hash-alg") || !obj.contains("primary-key-alg")
        || !obj.contains("pcr") || !obj.contains("pcr-bank")) {
        qCritical() << "Failed to get tpm algo from token.json!";
        return "";
    }

    QString sessionHashAlgo = obj.value("session-hash-alg").toString();
    QString sessionKeyAlgo = obj.value("session-key-alg").toString();
    QString primaryHashAlgo = obj.value("primary-hash-alg").toString();
    QString primaryKeyAlgo = obj.value("primary-key-alg").toString();
    QString pcr = obj.value("pcr").toString();
    QString pcrBank = obj.value("pcr-bank").toString();

    QVariantMap map {
        { "PropertyKey_EncryptType", (pin.isEmpty() ? kUseTpmAndPcr : kUseTpmAndPcrAndPin) },
        { "PropertyKey_SessionHashAlgo", (sessionHashAlgo.isEmpty() ? "sha256" : sessionHashAlgo) },
        { "PropertyKey_SessionKeyAlgo", (sessionKeyAlgo.isEmpty() ? "aes" : sessionKeyAlgo) },
        { "PropertyKey_PrimaryHashAlgo", primaryHashAlgo },
        { "PropertyKey_PrimaryKeyAlgo", primaryKeyAlgo },
        { "PropertyKey_Pcr", pcr},
        { "PropertyKey_PcrBank", pcrBank},
        { "PropertyKey_DirPath", dirPath }
    };
    if (!pin.isEmpty())
        map.insert("PropertyKey_PinCode", pin);

    QString passphrase;
    int ok = tpm_utils::decryptByTPM(map, &passphrase);
    if (ok != 0) {
        qWarning() << "cannot acquire passphrase from TPM for device"
                   << dev;
    }

    return passphrase;
}

bool tpm_passphrase_utils::tpmSupportInterAlgo()
{
    bool supportRSA { false };
    bool supportAES { false };
    bool supportSHA256 { false };

    tpm_utils::isSupportAlgoByTPM("rsa", &supportRSA);
    tpm_utils::isSupportAlgoByTPM("aes", &supportAES);
    tpm_utils::isSupportAlgoByTPM("sha256", &supportSHA256);

    return (supportRSA && supportAES && supportSHA256);
}

bool tpm_passphrase_utils::tpmSupportSMAlgo()
{
    bool supportSM3 { false };
    bool supportSM4 { false };

    tpm_utils::isSupportAlgoByTPM("sm3", &supportSM3);
    tpm_utils::isSupportAlgoByTPM("sm4", &supportSM4);

    return (supportSM3 && supportSM4);
}

bool tpm_passphrase_utils::getAlgorithm(QString *sessionHashAlgo, QString *sessionKeyAlgo,
                                        QString *primaryHashAlgo, QString *primaryKeyAlgo,
                                        QString *minorHashAlgo, QString *minorKeyAlgo,
                                        QString *pcr, QString *pcrbank)
{
    bool useAlgoFromDConfig = config_utils::enableAlgoFromDConfig();
    if (useAlgoFromDConfig) {
        if (config_utils::tpmAlgoFromDConfig(sessionHashAlgo, sessionKeyAlgo,
                                             primaryHashAlgo, primaryKeyAlgo,
                                             minorHashAlgo, minorKeyAlgo,
                                             pcr, pcrbank))
            return true;

        return false;
    }

    if (tpmSupportInterAlgo()) {
        (*sessionHashAlgo) = kTPMSessionHashAlgo;
        (*sessionKeyAlgo) = kTPMSessionKeyAlgo;
        (*primaryHashAlgo) = kTPMPrimaryHashAlgo;
        (*primaryKeyAlgo) = kTPMPrimaryKeyAlgo;
        (*minorHashAlgo) = kTPMMinorHashAlgo;
        (*minorKeyAlgo) = kTPMMinorKeyAlgo;
        (*pcr) = kPcr;
        (*pcrbank) = kTPMPcrBank;

        return true;
    }

    if (tpmSupportSMAlgo()) {
        (*sessionHashAlgo) = kTCMSessionHashAlgo;
        (*sessionKeyAlgo) = kTCMSessionKeyAlgo;
        (*primaryHashAlgo) = kTCMPrimaryHashAlgo;
        (*primaryKeyAlgo) = kTCMPrimaryKeyAlgo;
        (*minorHashAlgo) = kTCMMinorHashAlgo;
        (*minorKeyAlgo) = kTCMMinorKeyAlgo;
        (*pcr) = kPcr;
        (*pcrbank) = kTCMPcrBank;
        return true;
    }

    return false;
}

QString recovery_key_utils::formatRecoveryKey(const QString &raw)
{
    static const int kSectionLen = 6;
    QString formatted = raw;
    formatted.remove("-");
    int len = formatted.length();
    if (len > 24)
        formatted = formatted.mid(0, 24);

    len = formatted.length();
    int dashCount = len / kSectionLen;
    if (len % kSectionLen == 0)
        dashCount -= 1;
    for (; dashCount > 0; dashCount--)
        formatted.insert(dashCount * kSectionLen, '-');
    return formatted;
}

BlockDev device_utils::createBlockDevice(const QString &devObjPath)
{
    using namespace dfmmount;
    auto monitor = DDeviceManager::instance()->getRegisteredMonitor(DeviceType::kBlockDevice).objectCast<DBlockMonitor>();
    Q_ASSERT(monitor);
    return monitor->createDeviceById(devObjPath).objectCast<DBlockDevice>();
}

int dialog_utils::showDialog(const QString &title, const QString &msg, DialogType type)
{
    QString icon;
    switch (type) {
    case kInfo:
        icon = "dialog-information";
        break;
    case kWarning:
        icon = "dialog-warning";
        break;
    case kError:
        icon = "dialog-error";
        break;
    }
    Dtk::Widget::DDialog d;
    if (isWayland())
        d.setWindowFlag(Qt::WindowStaysOnTopHint);
    d.setTitle(title);
    d.setMessage(msg);
    d.setIcon(QIcon::fromTheme(icon));
    d.addButton(qApp->translate("dfmplugin_diskenc::ChgPassphraseDialog", "Confirm"));
    return d.exec();
}

void device_utils::cacheToken(const QString &device, const QVariantMap &token)
{
    if (token.isEmpty()) {
        QDir tmp("/tmp");
        tmp.rmpath(kGlobalTPMConfigPath + device);
        return;
    }

    auto makeFile = [](const QString &fileName, const QByteArray &content) {
        QFile f(fileName);
        if (!f.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
            qWarning() << "cannot cache token!" << fileName;
            return false;
        }

        f.write(content);
        f.flush();
        f.close();
        return true;
    };

    QString devTpmConfigPath = kGlobalTPMConfigPath + device;
    QDir tpmPath(devTpmConfigPath);
    if (!tpmPath.exists())
        tpmPath.mkpath(devTpmConfigPath);

    QJsonObject obj = QJsonObject::fromVariantMap(token);
    QJsonDocument doc(obj);
    QByteArray iv = obj.value("iv").toString().toLocal8Bit();
    QByteArray keyPriv = obj.value("kek-priv").toString().toLocal8Bit();
    QByteArray keyPub = obj.value("kek-pub").toString().toLocal8Bit();
    QByteArray cipher = obj.value("enc").toString().toLocal8Bit();
    iv = QByteArray::fromBase64(iv);
    keyPriv = QByteArray::fromBase64(keyPriv);
    keyPub = QByteArray::fromBase64(keyPub);
    cipher = QByteArray::fromBase64(cipher);

    bool ret = true;
    ret &= makeFile(devTpmConfigPath + "/token.json", doc.toJson());
    ret &= makeFile(devTpmConfigPath + "/iv.bin", iv);
    ret &= makeFile(devTpmConfigPath + "/key.priv", keyPriv);
    ret &= makeFile(devTpmConfigPath + "/key.pub", keyPub);
    ret &= makeFile(devTpmConfigPath + "/cipher.out", cipher);

    if (!ret)
        tpmPath.rmpath(devTpmConfigPath);
}

void dialog_utils::showTPMError(const QString &title, tpm_passphrase_utils::TPMError err)
{
    QString msg;
    switch (err) {
    case tpm_passphrase_utils::kTPMNoRandomNumber:
        msg = QObject::tr("Cannot generate random number by TPM");
        break;
    case tpm_passphrase_utils::kTPMMissingAlog:
        msg = QObject::tr("No available encrypt algorithm.");
        break;
    case tpm_passphrase_utils::kTPMEncryptFailed:
        msg = QObject::tr("TPM encrypt failed.");
        break;
    case tpm_passphrase_utils::kTPMLocked:
        msg = QObject::tr("TPM is locked.");
        break;
    default:
        break;
    }
    if (!msg.isEmpty())
        showDialog(title, msg, kError);
}

bool dialog_utils::isWayland()
{
    return QApplication::platformName() == "wayland";
}

bool config_utils::enableEncrypt()
{
    auto cfg = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                          "org.deepin.dde.file-manager.diskencrypt");
    cfg->deleteLater();
    return cfg->value("enableEncrypt", true).toBool();
}

bool config_utils::enableAlgoFromDConfig()
{
    auto cfg = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                          "org.deepin.dde.file-manager.diskencrypt");
    cfg->deleteLater();
    return cfg->value("enableUseTpmConfigAlgo", false).toBool();
}

bool config_utils::tpmAlgoFromDConfig(QString *sessionHash, QString *sessionKey,
                                      QString *primaryHash, QString *primaryKey,
                                      QString *minorHash, QString *minorKey,
                                      QString *pcr, QString *pcrBank)
{
    auto *cfg = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                           "org.deepin.dde.file-manager.diskencrypt");

    *sessionHash = cfg->value("tpmSessionHashAlgoName", "").toString();
    *sessionKey = cfg->value("tpmSessionKeyAlgoName", "").toString();
    *primaryHash = cfg->value("tpmPrimaryHashAlgoName", "").toString();
    *primaryKey = cfg->value("tpmPrimaryKeyAlgoName", "").toString();
    *minorHash = cfg->value("tpmMinorHashAlgoName", "").toString();
    *minorKey = cfg->value("tpmMinorKeyAlgoName", "").toString();
    *pcr = cfg->value("tpmPcr", "").toString();
    *pcrBank = cfg->value("tpmPcrBank", "").toString();

    delete cfg;

    if ((*sessionHash).isEmpty() || (*sessionKey).isEmpty()
        || (*primaryHash).isEmpty() || (*primaryKey).isEmpty()
        || (*minorHash).isEmpty() || (*minorKey).isEmpty()
        || (*pcr).isEmpty() || (*pcrBank).isEmpty())
        return false;

    return true;
}

int dialog_utils::showConfirmEncryptionDialog(const QString &device, bool needReboot)
{
    Dtk::Widget::DDialog dlg(qApp->activeWindow());
    if (isWayland())
        dlg.setWindowFlag(Qt::WindowStaysOnTopHint);
    dlg.setIcon(QIcon::fromTheme("drive-harddisk-root"));
    dlg.setTitle(QObject::tr("Confirm encrypt %1?").arg(device));
    QWidget *wid = new QWidget(&dlg);
    QVBoxLayout *lay = new QVBoxLayout(wid);
    QLabel *hint1 = new QLabel(QObject::tr("The current partition is about to be encrypted and cannot be canceled during "
                                           "the encryption process, please confirm the encryption."),
                               wid);
    hint1->setAlignment(Qt::AlignLeft);
    hint1->setWordWrap(true);
    lay->addWidget(hint1);

    QLabel *hint2 = new QLabel(QObject::tr("* After encrypting the partition, "
                                           "the system cannot be rolled back to a lower version, "
                                           "please confirm the encryption"),
                               wid);
    hint2->setAlignment(Qt::AlignLeft);
    hint2->setWordWrap(true);
    QPalette pal = hint2->palette();
    pal.setColor(QPalette::WindowText, QColor("red"));
    hint2->setPalette(pal);
    lay->addWidget(hint2);
    dlg.addContent(wid);
    dlg.addButton(QObject::tr("Cancel"));
    needReboot ? dlg.addButton(QObject::tr("Confirm and Reboot"), true, Dtk::Widget::DDialog::ButtonRecommend)
               : dlg.addButton(QObject::tr("Confirm"), true, Dtk::Widget::DDialog::ButtonRecommend);
    return dlg.exec();
}

int dialog_utils::showConfirmDecryptionDialog(const QString &device, bool needReboot)
{
    Dtk::Widget::DDialog dlg(qApp->activeWindow());
    if (isWayland())
        dlg.setWindowFlag(Qt::WindowStaysOnTopHint);
    dlg.setIcon(QIcon::fromTheme("drive-harddisk-root"));
    dlg.setTitle(QObject::tr("Decrypt %1?").arg(device));
    dlg.setMessage(QObject::tr("Decryption can take a long time, "
                               "so make sure power is connected until the decryption is complete."));
    dlg.addButton(QObject::tr("Cancel"));
    QString confirmTxt = needReboot ? QObject::tr("Confirm and Reboot") : QObject::tr("Confirm");
    dlg.addButton(confirmTxt, true, Dtk::Widget::DDialog::ButtonRecommend);
    return dlg.exec();
}

QString tpm_passphrase_utils::getPassphraseFromTPM_NonBlock(const QString &dev, const QString &pin)
{
    QEventLoop loop;
    QFutureWatcher<QString> watcher;
    QObject::connect(&watcher, &QFutureWatcher<QString>::finished, [&] { loop.exit(); });
    watcher.setFuture(QtConcurrent::run(tpm_passphrase_utils::getPassphraseFromTPM,
                                        dev,
                                        pin));

    qApp->setOverrideCursor(Qt::WaitCursor);
    loop.exec();
    qApp->restoreOverrideCursor();
    return watcher.result();
}

int tpm_passphrase_utils::genPassphraseFromTPM_NonBlock(const QString &dev, const QString &pin, QString *passphrase)
{
    QEventLoop loop;
    QFutureWatcher<int> watcher;
    QObject::connect(&watcher, &QFutureWatcher<int>::finished, [&] { loop.exit(); });
    watcher.setFuture(QtConcurrent::run(tpm_passphrase_utils::genPassphraseFromTPM,
                                        dev,
                                        pin,
                                        passphrase));

    qApp->setOverrideCursor(Qt::WaitCursor);
    loop.exec();
    qApp->restoreOverrideCursor();
    return watcher.result();
}
