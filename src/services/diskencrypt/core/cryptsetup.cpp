// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cryptsetup.h"
#include "diskencrypt_global.h"
#include "helpers/blockdevhelper.h"
#include "helpers/filesystemhelper.h"
#include "helpers/commonhelper.h"
#include "helpers/notificationhelper.h"
#include "helpers/cryptsetupcompabilityhelper.h"

#include <dfm-base/utils/finallyutil.h>

#include <QFile>
#include <QList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <libcryptsetup.h>

static constexpr char kDefaultPassphrase[] { "" };
static const int kDefaultPassphraseLen { 0 };

FILE_ENCRYPT_USE_NS

int crypt_setup::csInitEncrypt(const QString &dev, const QString &displayName, CryptPreProcessor *processor)
{
    qInfo() << "[crypt_setup::csInitEncrypt] Starting encryption initialization for device:" << dev << "display name:" << displayName;

    int r = crypt_setup_helper::initiable(dev);
    if (r < 0) {
        qCritical() << "[crypt_setup::csInitEncrypt] Device is not suitable for encryption:" << dev << "error:" << r;
        return r;
    }

    QString fileHeader;
    r = crypt_setup_helper::initEncryptHeaderFile(dev, displayName, processor, &fileHeader);
    if (r < 0) {
        qCritical() << "[crypt_setup::csInitEncrypt] Failed to initialize encryption header file for device:" << dev << "error:" << r;
        return r;
    }

    qInfo() << "[crypt_setup::csInitEncrypt] Encryption header file created successfully:" << fileHeader;
    r = crypt_setup_helper::initDeviceHeader(dev, fileHeader);
    if (r < 0) {
        qCritical() << "[crypt_setup::csInitEncrypt] Failed to initialize device header for device:" << dev << "error:" << r;
    } else {
        qInfo() << "[crypt_setup::csInitEncrypt] Encryption initialization completed successfully for device:" << dev;
    }
    return r;
}

int crypt_setup_helper::createHeaderFile(const QString &dev, QString *headerPath)
{
    qInfo() << "[crypt_setup_helper::createHeaderFile] Creating header file for device:" << dev;

    auto headerName = QString("%1_dfm_encrypt_header_%2.bin").arg(dev.mid(5)).arg(QDateTime::currentMSecsSinceEpoch());
    int fd = shm_open(headerName.toStdString().c_str(),
                      O_CREAT | O_EXCL | O_RDWR,
                      S_IRUSR | S_IWUSR);
    if (fd < 0) {
        qCritical() << "[crypt_setup_helper::createHeaderFile] Failed to create header file:" << headerName << "error:" << strerror(errno);
        return -1;
    }

    int r = ftruncate(fd, 32 * 1024 * 1024);   // allocate 32M space
    close(fd);
    if (r != 0) {
        qCritical() << "[crypt_setup_helper::createHeaderFile] Failed to allocate header file space:" << headerName << "error:" << strerror(errno);
        return -1;
    }

    if (headerPath) *headerPath = "/dev/shm/" + headerName;
    qInfo() << "[crypt_setup_helper::createHeaderFile] Header file created successfully:" << (*headerPath);
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::initiable(const QString &dev)
{
    qInfo() << "[crypt_setup_helper::initiable] Checking if device can be initialized for encryption:" << dev;

    auto ver = blockdev_helper::devCryptVersion(dev);
    if (ver != blockdev_helper::kNotEncrypted) {
        qWarning() << "[crypt_setup_helper::initiable] Device is already encrypted:" << dev << "version:" << ver;
        return -disk_encrypt::kErrorDeviceEncrypted;
    }

    auto ptr = blockdev_helper::createDevPtr(dev);
    if (ptr && !ptr->mountPoints().isEmpty()) {
        qWarning() << "[crypt_setup_helper::initiable] Device is currently mounted:" << dev << "mount points:" << ptr->mountPoints();
        return -disk_encrypt::kErrorDeviceMounted;
    }

    qInfo() << "[crypt_setup_helper::initiable] Device is suitable for encryption:" << dev;
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::initEncryptHeaderFile(const QString &dev, const QString &displayName,
                                              crypt_setup::CryptPreProcessor *processor,
                                              QString *fileHeader)
{
    qInfo() << "[crypt_setup_helper::initEncryptHeaderFile] Initializing encryption header file for device:" << dev;

    int r = 0;
    QString headerPath;
    r = crypt_setup_helper::createHeaderFile(dev, &headerPath);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::initEncryptHeaderFile] Failed to create header file for device:" << dev;
        return -disk_encrypt::kErrorCreateHeader;
    }

    // just use processor to determind whether to adjust the filesystem.
    // the normal initialize process passes a null processor and the usec-overlay mode passes a valid one.
    // so only normal process need to adjust fs.
    bool adjustFs = (processor == nullptr);
    qInfo() << "[crypt_setup_helper::initEncryptHeaderFile] Filesystem adjustment required:" << adjustFs << "for device:" << dev;

    // shrink file system leave space to hold crypt header.
    if (adjustFs && !filesystem_helper::shrinkFileSystem_ext(dev)) {
        qCritical() << "[crypt_setup_helper::initEncryptHeaderFile] Failed to shrink filesystem for device:" << dev;
        return -disk_encrypt::kErrorResizeFs;
    }

    struct crypt_device *cdev { nullptr };
    // do clean.
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev)
            crypt_free(cdev);
        if (r < 0) {
            qWarning() << "[crypt_setup_helper::initEncryptHeaderFile] Cleaning up due to error, removing header file:" << headerPath;
            ::remove(headerPath.toStdString().c_str());
            // filesystem_helper::expandFileSystem_ext(dev);
        }
    });

    r = crypt_init(&cdev,
                   headerPath.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::initEncryptHeaderFile] Failed to initialize crypt device for:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    crypt_set_rng_type(cdev, CRYPT_RNG_RANDOM);
    r = crypt_set_data_offset(cdev, 32 * 1024);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::initEncryptHeaderFile] Failed to set data offset for device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorSetOffset;
    }

    auto _dev = dev.toStdString();
    struct crypt_params_luks2 fmtArgs
    {
        .data_alignment = 0,
        .data_device = _dev.c_str(),
        .sector_size = 512,
        .label = nullptr,
        .subsystem = nullptr
    };
    auto cipher = common_helper::encryptCipher().toStdString();
    const char *mode = "xts-plain64";
    qInfo() << "[crypt_setup_helper::initEncryptHeaderFile] Formatting device with LUKS2, cipher:" << cipher.c_str() << "mode:" << mode;

    r = crypt_format(cdev,
                     CRYPT_LUKS2,
                     cipher.c_str(),
                     mode,
                     nullptr,
                     nullptr,
                     256 / 8,
                     &fmtArgs);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::initEncryptHeaderFile] Failed to format device with LUKS2:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorFormatLuks;
    }

    r = crypt_keyslot_add_by_volume_key(cdev,
                                        CRYPT_ANY_SLOT,
                                        nullptr,
                                        0,
                                        kDefaultPassphrase,
                                        kDefaultPassphraseLen);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::initEncryptHeaderFile] Failed to add empty keyslot for device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorAddKeyslot;
    }

    if (!displayName.isEmpty()) {
        r = crypt_set_label(cdev, displayName.toStdString().c_str(), nullptr);
        if (r < 0) {
            qWarning() << "[crypt_setup_helper::initEncryptHeaderFile] Failed to set label for device:" << dev << "label:" << displayName << "error:" << r << " (" << strerror(-r) << ")";
        } else {
            qInfo() << "[crypt_setup_helper::initEncryptHeaderFile] Label set successfully for device:" << dev << "label:" << displayName;
        }
    }

    struct crypt_params_luks2 luksArgs
    {
        .sector_size = 512
    };
    struct crypt_params_reencrypt encArgs
    {
        .mode = CRYPT_REENCRYPT_ENCRYPT,
        .direction = CRYPT_REENCRYPT_BACKWARD,
        .resilience = "datashift",
        .hash = "sha256",
        .data_shift = 32 * 1024,
        .max_hotzone_size = 0,
        .device_size = 0,
        .luks2 = &luksArgs,
        .flags = CRYPT_REENCRYPT_INITIALIZE_ONLY | CRYPT_REENCRYPT_MOVE_FIRST_SEGMENT
    };

    auto func = processor ? processor->proc : nullptr;
    auto argc = processor ? processor->argc : 0;
    auto argv = processor ? processor->argv : nullptr;
    auto init_by_passphrase = CryptSetupCompabilityHelper::instance()->initWithPreProcess();

    qInfo() << "[crypt_setup_helper::initEncryptHeaderFile] Initializing reencryption for device:" << dev << "with processor:" << (processor ? "enabled" : "disabled");

    if (init_by_passphrase) {
        r = init_by_passphrase(cdev,
                               nullptr,
                               "",
                               0,
                               CRYPT_ANY_SLOT,
                               0,
                               cipher.c_str(),
                               mode,
                               &encArgs,
                               func,
                               argc,
                               argv);
    } else {
        qWarning() << "[crypt_setup_helper::initEncryptHeaderFile] Compatibility helper not available, using fallback method for device:" << dev;
        r = crypt_reencrypt_init_by_passphrase(cdev,
                                               nullptr,
                                               "",
                                               0,
                                               CRYPT_ANY_SLOT,
                                               0,
                                               cipher.c_str(),
                                               mode,
                                               &encArgs);
    }
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::initEncryptHeaderFile] Cannot initialize reencrypt, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitReencrypt;
    }

    if (processor) {
        size_t keySize = crypt_get_volume_key_size(cdev);
        std::string key;
        key.resize(keySize);
        r = crypt_volume_key_get(cdev,
                                 0,
                                 key.data(),
                                 &keySize,
                                 nullptr,   // use null so that volume key can be get directly from cdev rather than calculate by passphrase
                                 kDefaultPassphraseLen);
        if (r < 0) {
            qCritical() << "[crypt_setup_helper::initEncryptHeaderFile] Cannot get volume key, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
            return -disk_encrypt::kErrorUnknown;
        }
        processor->volumeKey = key.data();
    }

    if (fileHeader) *fileHeader = headerPath;
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::initDeviceHeader(const QString &dev, const QString &fileHeader)
{
    qDebug() << "[crypt_setup_helper::initDeviceHeader] Initializing device header, device:" << dev << "header file:" << fileHeader;

    if (fileHeader.isEmpty() || !QFile(fileHeader).exists()) {
        qCritical() << "[crypt_setup_helper::initDeviceHeader] Header file does not exist, device:" << dev << "file:" << fileHeader;
        return -disk_encrypt::kErrorHeaderNotExist;
    }

    struct crypt_device *cdev { nullptr };
    int r = 0;
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev)
            crypt_free(cdev);
        ::remove(fileHeader.toStdString().c_str());
    });

    r = crypt_init(&cdev,
                   dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::initDeviceHeader] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_header_restore(cdev,
                             CRYPT_LUKS2,
                             fileHeader.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::initDeviceHeader] Cannot restore header from file, device:" << dev << "file:" << fileHeader << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorRestoreFromFile;
    }
    qInfo() << "[crypt_setup_helper::initDeviceHeader] Device header initialized successfully, device:" << dev;
    return disk_encrypt::kSuccess;
}

int crypt_setup::csResumeEncrypt(const QString &dev, const QString &activeName, const QString &displayName)
{
    qInfo() << "[crypt_setup::csResumeEncrypt] Starting encryption resume, device:" << dev << "activeName:" << activeName;

    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev) crypt_free(cdev);
    });

    int r = crypt_init_data_device(&cdev,
                                   dev.toStdString().c_str(),
                                   dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup::csResumeEncrypt] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csResumeEncrypt] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    uint32_t flags;
    r = crypt_persistent_flags_get(cdev,
                                   CRYPT_FLAGS_REQUIREMENTS,
                                   &flags);
    if (r < 0) {
        qCritical() << "[crypt_setup::csResumeEncrypt] Cannot get persistent flags, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorGetReencryptFlag;
    }
    if (!(flags & CRYPT_REQUIREMENT_ONLINE_REENCRYPT)) {
        qCritical() << "[crypt_setup::csResumeEncrypt] Flags do not satisfy resume requirements, device:" << dev << "flags:" << flags;
        return -disk_encrypt::kErrorWrongFlags;
    }

    crypt_params_reencrypt args;
    r = crypt_reencrypt_status(cdev,
                               &args);
    if (r == CRYPT_REENCRYPT_CRASH) {
        qInfo() << "[crypt_setup::csResumeEncrypt] Reencrypt crashed, attempting recovery for device:" << dev;
        // open and close to repair the decrypt process.
        auto name = "dm-" + dev.mid(5);
        r = crypt_activate_by_passphrase(cdev,
                                         name.toStdString().c_str(),
                                         CRYPT_ANY_SLOT,
                                         kDefaultPassphrase,
                                         kDefaultPassphraseLen,
                                         CRYPT_ACTIVATE_RECOVERY);
        if (r < 0) {
            qCritical() << "[crypt_setup::csResumeEncrypt] Cannot activate device for recovery, device:" << dev << "name:" << name << "error:" << r << " (" << strerror(-r) << ")";
            return -disk_encrypt::kErrorActive;
        }
        crypt_deactivate(cdev,
                         name.toStdString().c_str());
    }

    struct crypt_params_reencrypt encArgs
    {
        .mode = CRYPT_REENCRYPT_REENCRYPT,
        .direction = CRYPT_REENCRYPT_BACKWARD,
        .resilience = "datashift",
        .hash = "sha256",
        .data_shift = 32 * 1024,
        .max_hotzone_size = 0,
        .device_size = 0,
        .flags = CRYPT_REENCRYPT_RESUME_ONLY | CRYPT_REENCRYPT_MOVE_FIRST_SEGMENT
    };
    auto cname = activeName.toStdString();
    const char *name = activeName.isEmpty() ? nullptr : cname.c_str();
    r = crypt_reencrypt_init_by_passphrase(cdev,
                                           name,
                                           kDefaultPassphrase,
                                           kDefaultPassphraseLen,
                                           CRYPT_ANY_SLOT,
                                           CRYPT_ANY_SLOT,
                                           nullptr,
                                           nullptr,
                                           &encArgs);
    if (r < 0) {
        qCritical() << "[crypt_setup::csResumeEncrypt] Cannot initialize reencrypt, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitReencrypt;
    }

    qInfo() << "[crypt_setup::csResumeEncrypt] Processing encryption, device:" << dev;
    QPair<QString, QString> devInfo { dev, displayName };
    r = crypt_reencrypt_run(cdev,
                            crypt_setup_helper::onEncrypting,
                            (void *)&devInfo);
    qInfo() << "[crypt_setup::csResumeEncrypt] Encryption process finished, device:" << dev << "result:" << r;
    if (r < 0) {
        qCritical() << "[crypt_setup::csResumeEncrypt] Reencrypt failed, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorReencryptFailed;
    }
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::onEncrypting(uint64_t size, uint64_t offset, void *usrptr)
{
    auto dev = reinterpret_cast<QPair<QString, QString> *>(usrptr);
    Q_EMIT NotificationHelper::instance()->notifyEncryptProgress(dev->first,
                                                                 dev->second,
                                                                 double(1.0 * offset / size));
    return 0;
}

int crypt_setup_helper::onDecrypting(uint64_t size, uint64_t offset, void *usrptr)
{
    auto dev = reinterpret_cast<QPair<QString, QString> *>(usrptr);
    Q_EMIT NotificationHelper::instance()->notifyDecryptProgress(dev->first,
                                                                 dev->second,
                                                                 double(1.0 * offset / size));
    return 0;
}

int crypt_setup_helper::backupDetachHeader(const QString &dev, QString *fileHeader)
{
    qDebug() << "[crypt_setup_helper::backupDetachHeader] Backing up detach header, device:" << dev;

    QString headerPath;
    if (genDetachHeaderPath(dev, &headerPath) != disk_encrypt::kSuccess) {
        qCritical() << "[crypt_setup_helper::backupDetachHeader] Cannot generate detach header path, device:" << dev;
        return -disk_encrypt::kErrorUnknown;
    }

    struct crypt_device *cdev = nullptr;
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev) crypt_free(cdev);
    });

    if (QFile(headerPath).exists()) {
        qInfo() << "[crypt_setup_helper::backupDetachHeader] Backup header already exists:" << headerPath;
        if (fileHeader) *fileHeader = headerPath;
        return disk_encrypt::kSuccess;
    }

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::backupDetachHeader] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_header_backup(cdev,
                            nullptr,
                            headerPath.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::backupDetachHeader] Cannot backup device header, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorBackupHeader;
    }

    qInfo() << "[crypt_setup_helper::backupDetachHeader] Header backed up successfully to:" << headerPath;
    if (fileHeader) *fileHeader = headerPath;
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::headerStatus(const QString &fileHeader)
{
    qDebug() << "[crypt_setup_helper::headerStatus] Checking header status, file:" << fileHeader;

    QFile headerFile(fileHeader);
    if (!headerFile.open(QIODevice::ReadOnly)) {
        qCritical() << "[crypt_setup_helper::headerStatus] Cannot open header file:" << fileHeader;
        return kInvalidHeader;
    }

    headerFile.seek(4096);   // header json starts at 4096
    QString jsonStr = headerFile.readLine();
    headerFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toLocal8Bit());
    auto obj = doc.object();

    // read segments
    auto segments = obj.value("segments").toObject();
    if (segments.isEmpty()) {
        qWarning() << "[crypt_setup_helper::headerStatus] Segments not found in header:" << fileHeader;
        return kInvalidHeader;
    }
    bool hasLinear = false, hasCrypt = false;
    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        auto segment = iter.value().toObject();
        auto type = segment.value("type").toString();
        if (type == "linear")
            hasLinear = true;
        if (type == "crypt")
            hasCrypt = true;
    }

    // read mode
    QString mode;
    auto keyslots = obj.value("keyslots").toObject();
    for (auto iter = keyslots.begin(); iter != keyslots.end(); ++iter) {
        auto keyslot = iter.value().toObject();
        if (keyslot.value("type").toString() == "reencrypt") {
            mode = keyslot.value("mode").toString();
            break;
        }
    }

    qDebug() << "[crypt_setup_helper::headerStatus] Header analysis - mode:" << mode << "hasLinear:" << hasLinear << "hasCrypt:" << hasCrypt;

    if (mode == "encrypt") {
        if (hasLinear && hasCrypt)
            return kEncryptInProgress;
        if (hasLinear)
            return kEncryptInit;
    } else if (mode == "decrypt") {
        if (hasLinear && hasCrypt)
            return kDecryptInProgress;
        if (hasCrypt)
            return kDecryptInit;
    } else {
        if (hasLinear && !hasCrypt)
            return kDecryptFully;
        if (!hasLinear && hasCrypt)
            return kEncryptFully;
    }

    return kInvalidHeader;
}

int crypt_setup::csDecrypt(const QString &dev, const QString &passphrase, const QString &displayName, const QString &activeName)
{
    QString backupHeader;
    int r = crypt_setup_helper::backupDetachHeader(dev, &backupHeader);
    if (r < 0)
        return r;

    int status = crypt_setup_helper::headerStatus(backupHeader);
    if (status == crypt_setup_helper::kDecryptFully) {
        // device already decrypted, remove the backup header.
        ::remove(backupHeader.toStdString().c_str());
        return disk_encrypt::kSuccess;
    }

    struct crypt_device *cdev = nullptr;
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev) crypt_free(cdev);
    });

    r = crypt_init_data_device(&cdev,
                               backupHeader.toStdString().c_str(),
                               dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecrypt] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecrypt] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    uint32_t flags;
    r = crypt_persistent_flags_get(cdev,
                                   CRYPT_FLAGS_REQUIREMENTS,
                                   &flags);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecrypt] Cannot get persistent flags, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorGetReencryptFlag;
    }
    crypt_params_reencrypt args;
    r = crypt_reencrypt_status(cdev,
                               &args);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecrypt] Cannot get reencrypt status, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorCheckReencryptStatus;
    }

    bool encrypting = (flags & CRYPT_REQUIREMENT_OFFLINE_REENCRYPT)
            || (flags & CRYPT_REQUIREMENT_ONLINE_REENCRYPT);
    encrypting &= (args.mode == CRYPT_REENCRYPT_ENCRYPT);
    if (encrypting) {
        qCritical() << "[crypt_setup::csDecrypt] Device is under encryption, cannot decrypt, device:" << dev << "flags:" << flags;
        return -disk_encrypt::kErrorWrongFlags;
    }
    if (r == CRYPT_REENCRYPT_CRASH) {
        // open and close to repair the decrypt process.
        auto name = "dm-" + dev.mid(5);
        r = crypt_activate_by_passphrase(cdev,
                                         name.toStdString().c_str(),
                                         CRYPT_ANY_SLOT,
                                         passphrase.toStdString().c_str(),
                                         passphrase.length(),
                                         CRYPT_ACTIVATE_RECOVERY);
        if (r < 0) {
            qCritical() << "[crypt_setup::csDecrypt] Cannot activate device, device:" << dev << "name:" << name << "error:" << r << " (" << strerror(-r) << ")";
            return -disk_encrypt::kErrorActive;
        }
        crypt_deactivate(cdev,
                         name.toStdString().c_str());
    }

    bool resumeOnly = flags & CRYPT_REQUIREMENT_ONLINE_REENCRYPT;
    auto shift = crypt_get_data_offset(cdev);
    struct crypt_params_reencrypt encArgs
    {
        .mode = CRYPT_REENCRYPT_DECRYPT,
        .direction = CRYPT_REENCRYPT_FORWARD,
        .resilience = resumeOnly ? nullptr : "datashift-checksum",
        .hash = "sha256",
        .data_shift = shift,
        .max_hotzone_size = 0,
        .device_size = 0,
        .flags = resumeOnly ? CRYPT_REENCRYPT_RESUME_ONLY : CRYPT_REENCRYPT_MOVE_FIRST_SEGMENT
    };

    auto name = activeName.toStdString();
    r = crypt_reencrypt_init_by_passphrase(cdev,
                                           activeName.isEmpty() ? nullptr : name.c_str(),
                                           passphrase.toStdString().c_str(),
                                           passphrase.length(),
                                           CRYPT_ANY_SLOT,
                                           CRYPT_ANY_SLOT,
                                           nullptr,
                                           nullptr,
                                           &encArgs);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecrypt] Cannot initialize reencrypt process, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorWrongPassphrase;   // might not pass wrong.
    }

    qInfo() << "[crypt_setup::csDecrypt] Processing decryption, device:" << dev;
    QPair<QString, QString> devInfo { dev, displayName };
    r = crypt_reencrypt_run(cdev,
                            crypt_setup_helper::onDecrypting,
                            (void *)&devInfo);
    qInfo() << "[crypt_setup::csDecrypt] Decryption process finished, device:" << dev << "result:" << r;
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecrypt] Decrypt device failed, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorReencryptFailed;
    }

    remove(backupHeader.toStdString().c_str());
    // system("udevadm trigger");
    // system("udevadm settle");
    return disk_encrypt::kSuccess;
}

int crypt_setup::csAddPassphrase(const QString &dev, const QString &validPwd, const QString &newPwd)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });
    int r = crypt_init_data_device(&cdev,
                                   dev.toStdString().c_str(),
                                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csAddPassphrase] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csAddPassphrase] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    r = crypt_keyslot_add_by_passphrase(cdev,
                                        CRYPT_ANY_SLOT,
                                        validPwd.toStdString().c_str(),
                                        validPwd.length(),
                                        newPwd.toStdString().c_str(),
                                        newPwd.length());
    if (r < 0) {
        qCritical() << "[crypt_setup::csAddPassphrase] Change passphrase failed, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorAddKeyslot;
    }
    return r;   // the key slot number.
}

int crypt_setup::csChangePassphrase(const QString &dev, const QString &oldPwd, const QString &newPwd)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });
    int r = crypt_init_data_device(&cdev,
                                   dev.toStdString().c_str(),
                                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csChangePassphrase] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csChangePassphrase] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    r = crypt_keyslot_change_by_passphrase(cdev,
                                           CRYPT_ANY_SLOT,
                                           CRYPT_ANY_SLOT,
                                           oldPwd.toStdString().c_str(),
                                           oldPwd.length(),
                                           newPwd.toStdString().c_str(),
                                           newPwd.length());
    if (r < 0) {
        qCritical() << "[crypt_setup::csChangePassphrase] Change passphrase failed, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorChangePassphraseFailed;
    }
    return r;   // the key slot number.
}

int crypt_setup::csRemoveKeyslot(const QString &dev, int keyslot)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });
    int r = crypt_init_data_device(&cdev,
                                   dev.toStdString().c_str(),
                                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csRemoveKeyslot] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csRemoveKeyslot] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    r = crypt_keyslot_destroy(cdev, keyslot);
    if (r < 0) {
        qCritical() << "[crypt_setup::csRemoveKeyslot] Remove keyslot failed, device:" << dev << "keyslot:" << keyslot << "error:" << r << " (" << strerror(-r) << ")";
        return r;
    }

    qInfo() << "[crypt_setup::csRemoveKeyslot] Keyslot removed successfully, device:" << dev << "keyslot:" << keyslot;
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::setToken(const QString &dev, const QString &token)
{
    if (token.isEmpty())
        return disk_encrypt::kSuccess;

    QJsonDocument doc = QJsonDocument::fromJson(token.toLocal8Bit());
    QJsonObject obj = doc.object();
    int tokenIndex = obj.value("token_index").toInt(CRYPT_ANY_TOKEN);

    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::setToken] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::setToken] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }
    r = crypt_token_json_set(cdev,
                             tokenIndex,
                             token.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::setToken] Cannot set token, device:" << dev << "index:" << tokenIndex << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorSetTokenFailed;
    }
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::getToken(const QString &dev, QString *token)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::getToken] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::getToken] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    for (int i = 0; i < 32 /* LUKS2_TOKENS_MAX */; ++i) {
        const char *tokenStr { nullptr };
        if ((r = crypt_token_json_get(cdev, i, &tokenStr)) < 0)
            continue;
        QString json(tokenStr);
        if (json.contains("usec-tpm2")) {
            QJsonDocument doc = QJsonDocument::fromJson(tokenStr);
            QJsonObject obj = doc.object();
            obj.insert("token_index", i);
            doc.setObject(obj);
            if (token) *token = doc.toJson();
            return disk_encrypt::kSuccess;
        }
    }

    qInfo() << "[crypt_setup_helper::getToken] Token not found, device:" << dev;
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::getRecoveryKeySlots(const QString &dev, QList<int> *keySlots)
{
    if (!keySlots) {
        qCritical() << "[crypt_setup_helper::getRecoveryKeySlots] KeySlots parameter is null";
        return -disk_encrypt::kErrorUnknown;
    }

    keySlots->clear();

    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::getRecoveryKeySlots] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::getRecoveryKeySlots] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    // 遍历所有 token，找到 usec-recoverykey 类型的
    for (int i = 0; i < 32 /* LUKS2_TOKENS_MAX */; ++i) {
        const char *tokenStr { nullptr };
        if ((r = crypt_token_json_get(cdev, i, &tokenStr)) < 0)
            continue;

        QString json(tokenStr);
        if (json.contains("usec-recoverykey")) {
            // 解析 token 中的 keyslots 字段
            QJsonDocument doc = QJsonDocument::fromJson(tokenStr);
            QJsonObject obj = doc.object();
            QJsonArray keyslots = obj.value("keyslots").toArray();

            for (const auto &slot : keyslots) {
                bool ok = false;
                int slotNum = slot.toString().toInt(&ok);
                if (ok) {
                    keySlots->append(slotNum);
                    qInfo() << "[crypt_setup_helper::getRecoveryKeySlots] Found recovery key in slot:" << slotNum;
                }
            }
        }
    }

    if (keySlots->isEmpty()) {
        qInfo() << "[crypt_setup_helper::getRecoveryKeySlots] No recovery key slots found, device:" << dev;
    }

    return disk_encrypt::kSuccess;
}

int crypt_setup::csActivateDevice(const QString &dev, const QString &activateName, const QString &passphrase)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup::csActivateDevice] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csActivateDevice] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    r = crypt_status(cdev,
                     activateName.toStdString().c_str());
    if (r == CRYPT_INACTIVE) {
        r = crypt_activate_by_passphrase(cdev,
                                         activateName.toStdString().c_str(),
                                         CRYPT_ANY_SLOT,
                                         passphrase.toStdString().c_str(),
                                         passphrase.length(),
                                         CRYPT_ACTIVATE_SHARED);
        if (r < 0) {
            qCritical() << "[crypt_setup::csActivateDevice] Cannot activate device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
            return -disk_encrypt::kErrorActive;
        }
    }
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::encryptStatus(const QString &dev)
{
    int status = disk_encrypt::kStatusUnknown;
    auto ver = blockdev_helper::devCryptVersion(dev);
    if (ver == blockdev_helper::kNotEncrypted)
        return disk_encrypt::kStatusNotEncrypted;

    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });

    int r = 0;
    QString backupPath;
    r = genDetachHeaderPath(dev, &backupPath);
    if (r < 0)
        return r;

    if (QFile(backupPath).exists()) {
        r = crypt_init_data_device(&cdev,
                                   backupPath.toStdString().c_str(),
                                   dev.toStdString().c_str());
    } else {
        r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    }
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::encryptStatus] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::encryptStatus] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    crypt_params_reencrypt args;
    r = crypt_reencrypt_status(cdev,
                               &args);
    if (r == CRYPT_REENCRYPT_NONE)
        return disk_encrypt::kStatusFinished;
    if (args.mode == CRYPT_REENCRYPT_ENCRYPT)
        status = disk_encrypt::kStatusEncrypt;
    else if (args.mode == CRYPT_REENCRYPT_DECRYPT)
        status = disk_encrypt::kStatusDecrypt;

    uint32_t flags;
    r = crypt_persistent_flags_get(cdev,
                                   CRYPT_FLAGS_REQUIREMENTS,
                                   &flags);
    if (r < 0) {
        qCritical() << "[crypt_setup_helper::encryptStatus] Cannot get persistent flags, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorGetReencryptFlag;
    }
    if (flags & CRYPT_REQUIREMENT_OFFLINE_REENCRYPT)
        status |= disk_encrypt::kStatusOffline;
    if (flags & CRYPT_REQUIREMENT_ONLINE_REENCRYPT)
        status |= disk_encrypt::kStatusOnline;
    if (flags & CRYPT_REQUIREMENT_UNKNOWN)
        status = disk_encrypt::kStatusUnknown;
    return status;
}

int crypt_setup::csActivateDeviceByVolume(const QString &dev, const QString &activateName, const QByteArray &volume)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup::csActivateDeviceByVolume] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csActivateDeviceByVolume] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    r = crypt_status(cdev,
                     activateName.toStdString().c_str());
    if (r == CRYPT_INACTIVE) {
        r = crypt_activate_by_volume_key(cdev,
                                         activateName.toStdString().c_str(),
                                         volume.data(),
                                         volume.length(),
                                         CRYPT_ACTIVATE_SHARED);
        if (r < 0) {
            qCritical() << "[crypt_setup::csActivateDeviceByVolume] Cannot activate device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
            return -disk_encrypt::kErrorActive;
        }
    }
    return disk_encrypt::kSuccess;
}

int crypt_setup::csSetLabel(const QString &dev, const QString &label)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup::csSetLabel] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev, CRYPT_LUKS, nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csSetLabel] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    r = crypt_set_label(cdev, label.toStdString().c_str(), nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csSetLabel] Cannot set label on device, label:" << label << "device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorSetLabel;
    }
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::genDetachHeaderPath(const QString &dev, QString *name)
{
    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qCritical() << "[crypt_setup_helper::genDetachHeaderPath] Cannot create device object, device:" << dev;
        return -disk_encrypt::kErrorUnknown;
    }

    auto puuid = ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    if (puuid.isEmpty())
        puuid = dev.mid(5);
    if (name)
        *name = kUSecDetachHeaderPrefix + puuid + ".bin";
    name->prepend(QString(kUSecBootRoot) + "/");

    return disk_encrypt::kSuccess;
}

int crypt_setup::csDecryptMoveHead(const QString &dev, const QString &passphrase, const QString &displayName)
{
    QString backupHeader;
    int r = crypt_setup_helper::backupDetachHeader(dev, &backupHeader);
    if (r < 0)
        return r;

    int status = crypt_setup_helper::headerStatus(backupHeader);
    if (status == crypt_setup_helper::kDecryptFully) {
        // device already decrypted, remove the backup header.
        ::remove(backupHeader.toStdString().c_str());
        return disk_encrypt::kSuccess;
    }

    struct crypt_device *cdev = nullptr;
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev) crypt_free(cdev);
    });

    r = crypt_init_data_device(&cdev,
                               backupHeader.toStdString().c_str(),
                               dev.toStdString().c_str());
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecryptMoveHead] Cannot initialize crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecryptMoveHead] Cannot load crypt device, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorLoadCrypt;
    }

    uint32_t flags;
    r = crypt_persistent_flags_get(cdev,
                                   CRYPT_FLAGS_REQUIREMENTS,
                                   &flags);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecryptMoveHead] Cannot get persistent flags, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorGetReencryptFlag;
    }
    crypt_params_reencrypt args;
    r = crypt_reencrypt_status(cdev,
                               &args);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecryptMoveHead] Cannot get reencrypt status, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorCheckReencryptStatus;
    }

    bool encrypting = (flags & CRYPT_REQUIREMENT_OFFLINE_REENCRYPT)
            || (flags & CRYPT_REQUIREMENT_ONLINE_REENCRYPT);
    encrypting &= (args.mode == CRYPT_REENCRYPT_ENCRYPT);
    if (encrypting) {
        qCritical() << "[crypt_setup::csDecryptMoveHead] Device is under encryption, cannot decrypt, device:" << dev << "flags:" << flags;
        return -disk_encrypt::kErrorWrongFlags;
    }
    if (r == CRYPT_REENCRYPT_CRASH) {
        // open and close to repair the decrypt process.
        auto name = "dm-" + dev.mid(5);
        r = crypt_activate_by_passphrase(cdev,
                                         name.toStdString().c_str(),
                                         CRYPT_ANY_SLOT,
                                         passphrase.toStdString().c_str(),
                                         passphrase.length(),
                                         CRYPT_ACTIVATE_RECOVERY);
        if (r < 0) {
            qCritical() << "[crypt_setup::csDecryptMoveHead] Cannot activate device, device:" << dev << "name:" << name << "error:" << r << " (" << strerror(-r) << ")";
            return -disk_encrypt::kErrorActive;
        }
        crypt_deactivate(cdev,
                         name.toStdString().c_str());
    }

    struct crypt_params_reencrypt encArgs
    {
        .mode = CRYPT_REENCRYPT_DECRYPT,
        .direction = CRYPT_REENCRYPT_BACKWARD,
        .resilience = "checksum",
        .hash = "sha256",
        .data_shift = 0,
        .max_hotzone_size = 0,
        .device_size = 0
    };

    r = crypt_reencrypt_init_by_passphrase(cdev,
                                           nullptr,
                                           passphrase.toStdString().c_str(),
                                           passphrase.length(),
                                           CRYPT_ANY_SLOT,
                                           CRYPT_ANY_SLOT,
                                           nullptr,
                                           nullptr,
                                           &encArgs);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecryptMoveHead] Cannot initialize reencrypt process, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorWrongPassphrase;   // might not pass wrong.
    }

    QPair<QString, QString> devInfo { dev, displayName };
    r = crypt_reencrypt_run(cdev,
                            crypt_setup_helper::onDecrypting,
                            (void *)&devInfo);
    if (r < 0) {
        qCritical() << "[crypt_setup::csDecryptMoveHead] Decrypt device failed, device:" << dev << "error:" << r << " (" << strerror(-r) << ")";
        return -disk_encrypt::kErrorReencryptFailed;
    }

    qInfo() << "[crypt_setup::csDecryptMoveHead] Start moving device superblock area";
    Q_EMIT NotificationHelper::instance()->notifyDecryptProgress(dev, displayName, 0.99);
    if (!filesystem_helper::moveFsForward(dev)) {
        qCritical() << "[crypt_setup::csDecryptMoveHead] Recovery filesystem failed, device:" << dev;
        return -disk_encrypt::kErrorResizeFs;
    }
    Q_EMIT NotificationHelper::instance()->notifyDecryptProgress(dev, displayName, 1);
    qInfo() << "[crypt_setup::csDecryptMoveHead] Device superblock has been moved successfully";

    ::remove(backupHeader.toStdString().c_str());

    return disk_encrypt::kSuccess;
}
