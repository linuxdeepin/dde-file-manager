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
        QDBusReply<QString> reply = iface.call("QueryTPMToken", dev);
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

    QString sessionHashAlgo, sessionKeyAlgo, primaryHashAlgo, primaryKeyAlgo, minorHashAlgo, minorKeyAlgo;
    if (!getAlgorithm(&sessionHashAlgo, &sessionKeyAlgo, &primaryHashAlgo, &primaryKeyAlgo, &minorHashAlgo, &minorKeyAlgo)) {
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
        { "PropertyKey_DirPath", dirPath },
        { "PropertyKey_Plain", *passphrase },
    };
    if (pin.isEmpty()) {
        map.insert("PropertyKey_EncryptType", kUseTpmAndPcr);
        map.insert("PropertyKey_Pcr", "7");
        map.insert("PropertyKey_PcrBank", primaryHashAlgo);
    } else {
        map.insert("PropertyKey_EncryptType", kUseTpmAndPrcAndPin);
        map.insert("PropertyKey_Pcr", "7");
        map.insert("PropertyKey_PcrBank", primaryHashAlgo);
        map.insert("PropertyKey_PinCode", pin);
    }

    int err = tpm_utils::encryptByTPM(map);
    if (err != 0) {
        qCritical() << "save to TPM failed!!!";
        return TPMError(err);
    }

    QSettings settings(dirPath + QDir::separator() + "algo.ini", QSettings::IniFormat);
    settings.setValue(kConfigKeySessionHashAlgo, QVariant(sessionHashAlgo));
    settings.setValue(kConfigKeyPriKeyAlgo, QVariant(sessionKeyAlgo));
    settings.setValue(kConfigKeyPriHashAlgo, QVariant(primaryHashAlgo));
    settings.setValue(kConfigKeyPriKeyAlgo, QVariant(primaryKeyAlgo));

    return kTPMNoError;
}

QString tpm_passphrase_utils::getPassphraseFromTPM(const QString &dev, const QString &pin)
{
    const QString dirPath = kGlobalTPMConfigPath + dev;
    QSettings tpmSets(dirPath + QDir::separator() + "algo.ini", QSettings::IniFormat);
    const QString sessionHashAlgo = tpmSets.value(kConfigKeySessionHashAlgo).toString();
    const QString sessionKeyAlgo = tpmSets.value(kConfigKeySessionKeyAlgo).toString();
    const QString primaryHashAlgo = tpmSets.value(kConfigKeyPriHashAlgo).toString();
    const QString primaryKeyAlgo = tpmSets.value(kConfigKeyPriKeyAlgo).toString();
    QVariantMap map {
        { "PropertyKey_EncryptType", (pin.isEmpty() ? kUseTpmAndPcr : kUseTpmAndPrcAndPin) },
        { "PropertyKey_SessionHashAlgo", (sessionHashAlgo.isEmpty() ? "sha256" : sessionHashAlgo) },   // TODO:gongheng need help by liangbo
        { "PropertyKey_SessionKeyAlgo", (sessionKeyAlgo.isEmpty() ? "aes" : sessionKeyAlgo) },
        { "PropertyKey_PrimaryHashAlgo", primaryHashAlgo },
        { "PropertyKey_PrimaryKeyAlgo", primaryKeyAlgo },
        { "PropertyKey_DirPath", dirPath }
    };

    QString tokenDocPath = dirPath + QDir::separator() + "token.json";
    QFile file(tokenDocPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open token.json!";
        return "";
    }
    QJsonDocument tokenDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject obj = tokenDoc.object();
    if (!obj.contains("pcr") || !obj.contains("pcr-bank")) {
        qCritical() << "Failed to get pcr or pcr-bank from token.json!";
        return "";
    }
    const QString pcr = obj.value("pcr").toString();
    const QString pcr_bank = obj.value("pcr-bank").toString();
    if (!pin.isEmpty())
        map.insert("PropertyKey_PinCode", pin);
    map.insert("PropertyKey_Pcr", pcr);
    map.insert("PropertyKey_PcrBank", pcr_bank);

    QString passphrase;
    int ok = tpm_utils::decryptByTPM(map, &passphrase);
    if (ok != 0) {
        qWarning() << "cannot acquire passphrase from TPM for device"
                   << dev;
    }

    return passphrase;
}

bool tpm_passphrase_utils::getAlgorithm(QString *sessionHashAlgo, QString *sessionKeyAlgo,
                                        QString *primaryHashAlgo, QString *primaryKeyAlgo,
                                        QString *minorHashAlgo, QString *minorKeyAlgo)
{
    bool re1 { false };
    bool re2 { false };
    bool re3 { false };
    bool re4 { false };
    bool re5 { false };
    bool re6 { false };
    tpm_utils::isSupportAlgoByTPM(kTPMSessionHashAlgo, &re1);
    tpm_utils::isSupportAlgoByTPM(kTPMSessionKeyAlgo, &re2);
    tpm_utils::isSupportAlgoByTPM(kTPMPrimaryHashAlgo, &re3);
    tpm_utils::isSupportAlgoByTPM(kTPMPrimaryKeyAlgo, &re4);
    tpm_utils::isSupportAlgoByTPM(kTPMMinorHashAlgo, &re5);
    tpm_utils::isSupportAlgoByTPM(kTPMMinorKeyAlgo, &re6);

    if (re1 && re2 && re3 && re4 && re5 && re6) {
        (*sessionHashAlgo) = kTPMSessionHashAlgo;
        (*sessionKeyAlgo) = kTPMSessionKeyAlgo;
        (*primaryHashAlgo) = kTPMPrimaryHashAlgo;
        (*primaryKeyAlgo) = kTPMPrimaryKeyAlgo;
        (*minorHashAlgo) = kTPMMinorHashAlgo;
        (*minorKeyAlgo) = kTPMMinorKeyAlgo;
        return true;
    }

    re1 = false;
    re2 = false;
    re3 = false;
    re4 = false;
    re5 = false;
    re6 = false;
    tpm_utils::isSupportAlgoByTPM(kTCMSessionHashAlgo, &re1);
    tpm_utils::isSupportAlgoByTPM(kTCMSessionKeyAlgo, &re2);
    tpm_utils::isSupportAlgoByTPM(kTCMPrimaryHashAlgo, &re3);
    tpm_utils::isSupportAlgoByTPM(kTCMPrimaryKeyAlgo, &re4);
    tpm_utils::isSupportAlgoByTPM(kTCMMinorHashAlgo, &re5);
    tpm_utils::isSupportAlgoByTPM(kTCMMinorKeyAlgo, &re6);

    if (re1 && re2 && re3 && re4 && re5 && re6) {
        (*sessionHashAlgo) = kTCMSessionHashAlgo;
        (*sessionKeyAlgo) = kTCMSessionKeyAlgo;
        (*primaryHashAlgo) = kTCMPrimaryHashAlgo;
        (*primaryKeyAlgo) = kTCMPrimaryKeyAlgo;
        (*minorHashAlgo) = kTCMMinorHashAlgo;
        (*minorKeyAlgo) = kTCMMinorKeyAlgo;
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

    QSettings algo(devTpmConfigPath + "/algo.ini", QSettings::IniFormat);
    algo.setValue("session_hash_algo", obj.value("session-hash-alg").toString());
    algo.setValue("session_key_algo", obj.value("session-key-alg").toString());
    algo.setValue("primary_hash_algo", obj.value("primary-hash-alg").toString());
    algo.setValue("primary_key_algo", obj.value("primary-key-alg").toString());

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
