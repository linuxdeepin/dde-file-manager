// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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

int crypt_setup::csInitEncrypt(const QString &dev, CryptPreProcessor *processor)
{
    int r = crypt_setup_helper::initiable(dev);
    if (r < 0) return r;

    QString fileHeader;
    r = crypt_setup_helper::initEncryptHeaderFile(dev, processor, &fileHeader);
    if (r < 0) return r;

    r = crypt_setup_helper::initDeviceHeader(dev, fileHeader);
    return r;
}

int crypt_setup_helper::createHeaderFile(const QString &dev, QString *headerPath)
{
    auto headerName = QString("%1_dfm_encrypt_header_%2.bin").arg(dev.mid(5)).arg(QDateTime::currentMSecsSinceEpoch());
    int fd = shm_open(headerName.toStdString().c_str(),
                      O_CREAT | O_EXCL | O_RDWR,
                      S_IRUSR | S_IWUSR);
    if (fd < 0) {
        qWarning() << "cannot create header file at" << headerName << strerror(errno);
        return -1;
    }
    int r = ftruncate(fd, 32 * 1024 * 1024);   // allocate 32M space
    close(fd);
    if (r != 0) {
        qWarning() << "cannot allocate header file space!" << headerName;
        return -1;
    }

    if (headerPath) *headerPath = "/dev/shm/" + headerName;
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::initiable(const QString &dev)
{
    auto ver = blockdev_helper::devCryptVersion(dev);
    if (ver != blockdev_helper::kNotEncrypted) {
        qWarning() << "device is already inited!" << dev;
        return -disk_encrypt::kErrorDeviceEncrypted;
    }

    auto ptr = blockdev_helper::createDevPtr(dev);
    if (ptr && !ptr->mountPoints().isEmpty()) {
        qWarning() << "device is mounted!" << dev;
        return -disk_encrypt::kErrorDeviceMounted;
    }
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::initEncryptHeaderFile(const QString &dev,
                                              crypt_setup::CryptPreProcessor *processor,
                                              QString *fileHeader)
{
    int r = 0;
    QString headerPath;
    r = crypt_setup_helper::createHeaderFile(dev, &headerPath);
    if (r < 0) {
        qWarning() << "cannot allocate header file!" << dev;
        return -disk_encrypt::kErrorCreateHeader;
    }

    // just use processor to determind whether to adjust the filesystem.
    // the normal initialize process passes a null processor and the usec-overlay mode passes a valid one.
    // so only normal process need to adjust fs.
    bool adjustFs = (processor == nullptr);
    // shrink file system leave space to hold crypt header.
    if (adjustFs && !filesystem_helper::shrinkFileSystem_ext(dev)) {
        qWarning() << "cannot resize filesytem!" << dev;
        return -disk_encrypt::kErrorResizeFs;
    }

    struct crypt_device *cdev { nullptr };
    // do clean.
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev)
            crypt_free(cdev);
        if (r < 0) {
            ::remove(headerPath.toStdString().c_str());
            // filesystem_helper::expandFileSystem_ext(dev);
        }
    });

    r = crypt_init(&cdev,
                   headerPath.toStdString().c_str());
    if (r < 0) {
        qWarning() << "init cdev failed!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    crypt_set_rng_type(cdev, CRYPT_RNG_RANDOM);
    r = crypt_set_data_offset(cdev, 32 * 1024);
    if (r < 0) {
        qWarning() << "set cdev offset failed!" << dev << r;
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
    r = crypt_format(cdev,
                     CRYPT_LUKS2,
                     cipher.c_str(),
                     mode,
                     nullptr,
                     nullptr,
                     256 / 8,
                     &fmtArgs);
    if (r < 0) {
        qWarning() << "format device failed!" << dev << r;
        return -disk_encrypt::kErrorFormatLuks;
    }

    r = crypt_keyslot_add_by_volume_key(cdev,
                                        CRYPT_ANY_SLOT,
                                        nullptr,
                                        0,
                                        kDefaultPassphrase,
                                        kDefaultPassphraseLen);
    if (r < 0) {
        qWarning() << "cannot add empty keyslot!" << dev << r;
        return -disk_encrypt::kErrorAddKeyslot;
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
        qWarning() << "cannot init reencrypt!" << dev << r;
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
            qWarning() << "cannot get volume key!" << dev << r;
            return -disk_encrypt::kErrorUnknown;
        }
        processor->volumeKey = key.data();
    }

    if (fileHeader) *fileHeader = headerPath;
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::initDeviceHeader(const QString &dev, const QString &fileHeader)
{
    if (fileHeader.isEmpty() || !QFile(fileHeader).exists()) {
        qWarning() << "header file not exist!" << dev << fileHeader;
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
        qWarning() << "cannot init crypt!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_header_restore(cdev,
                             CRYPT_LUKS2,
                             fileHeader.toStdString().c_str());
    if (r < 0) {
        qWarning() << "cannot restore header from file!" << dev << r << fileHeader;
        return -disk_encrypt::kErrorRestoreFromFile;
    }
    return disk_encrypt::kSuccess;
}

int crypt_setup::csResumeEncrypt(const QString &dev, const QString &activeName, const QString &displayName)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev) crypt_free(cdev);
    });

    int r = crypt_init_data_device(&cdev,
                                   dev.toStdString().c_str(),
                                   dev.toStdString().c_str());
    if (r < 0) {
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
        return -disk_encrypt::kErrorLoadCrypt;
    }

    uint32_t flags;
    r = crypt_persistent_flags_get(cdev,
                                   CRYPT_FLAGS_REQUIREMENTS,
                                   &flags);
    if (r < 0) {
        qWarning() << "cannot get persistent flag!" << dev << r;
        return -disk_encrypt::kErrorGetReencryptFlag;
    }
    if (!(flags & CRYPT_REQUIREMENT_ONLINE_REENCRYPT)) {
        qWarning() << "flag is not satisfy for resume!" << dev << r << flags;
        return -disk_encrypt::kErrorWrongFlags;
    }

    crypt_params_reencrypt args;
    r = crypt_reencrypt_status(cdev,
                               &args);
    if (r == CRYPT_REENCRYPT_CRASH) {
        // open and close to repair the decrypt process.
        auto name = "dm-" + dev.mid(5);
        r = crypt_activate_by_passphrase(cdev,
                                         name.toStdString().c_str(),
                                         CRYPT_ANY_SLOT,
                                         kDefaultPassphrase,
                                         kDefaultPassphraseLen,
                                         CRYPT_ACTIVATE_RECOVERY);
        if (r < 0) {
            qWarning() << "cannot activate device by name!" << dev << r << name;
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
        qWarning() << "cannot init reencrypt!" << dev << r;
        return -disk_encrypt::kErrorInitReencrypt;
    }

    qInfo() << "processing encryption..." << dev;
    QPair<QString, QString> devInfo { dev, displayName };
    r = crypt_reencrypt_run(cdev,
                            crypt_setup_helper::onEncrypting,
                            (void *)&devInfo);
    qInfo() << "encryption process finished" << dev << r;
    if (r < 0) {
        qWarning() << "run reencrypt failed!" << dev << r;
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
    QString headerPath;
    if (genDetachHeaderPath(dev, &headerPath) != disk_encrypt::kSuccess)
        return -disk_encrypt::kErrorUnknown;

    struct crypt_device *cdev = nullptr;
    dfmbase::FinallyUtil atFinish([&] {
        if (cdev) crypt_free(cdev);
    });

    if (QFile(headerPath).exists()) {
        qInfo() << "backup header already exists.";
        if (fileHeader) *fileHeader = headerPath;
        return disk_encrypt::kSuccess;
    }

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_header_backup(cdev,
                            nullptr,
                            headerPath.toStdString().c_str());
    if (r < 0) {
        qWarning() << "cannot backup device header!" << dev << r;
        return -disk_encrypt::kErrorBackupHeader;
    }

    if (fileHeader) *fileHeader = headerPath;
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::headerStatus(const QString &fileHeader)
{
    QFile headerFile(fileHeader);
    if (!headerFile.open(QIODevice::ReadOnly)) {
        qWarning() << "open header failed!";
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
        qWarning() << "segments not found";
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
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
        return -disk_encrypt::kErrorLoadCrypt;
    }

    uint32_t flags;
    r = crypt_persistent_flags_get(cdev,
                                   CRYPT_FLAGS_REQUIREMENTS,
                                   &flags);
    if (r < 0) {
        qWarning() << "cannot get persistent flag!" << dev << r;
        return -disk_encrypt::kErrorGetReencryptFlag;
    }
    crypt_params_reencrypt args;
    r = crypt_reencrypt_status(cdev,
                               &args);
    if (r < 0) {
        qWarning() << "get reencrypt status failed!" << dev << r;
        return -disk_encrypt::kErrorCheckReencryptStatus;
    }

    bool encrypting = (flags & CRYPT_REQUIREMENT_OFFLINE_REENCRYPT)
            || (flags & CRYPT_REQUIREMENT_ONLINE_REENCRYPT);
    encrypting &= (args.mode == CRYPT_REENCRYPT_ENCRYPT);
    if (encrypting) {
        qWarning() << "device is under encrypting, cannot decrypt!" << dev << flags;
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
            qWarning() << "cannot activate device by name!" << dev << r << name;
            return -disk_encrypt::kErrorActive;
        }
        crypt_deactivate(cdev,
                         name.toStdString().c_str());
    }

    auto shift = crypt_get_data_offset(cdev);
    struct crypt_params_reencrypt encArgs
    {
        .mode = CRYPT_REENCRYPT_DECRYPT,
        .direction = CRYPT_REENCRYPT_FORWARD,
        .resilience = "datashift-checksum",
        .hash = "sha256",
        .data_shift = shift,
        .max_hotzone_size = 0,
        .device_size = 0,
        .flags = CRYPT_REENCRYPT_MOVE_FIRST_SEGMENT
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
        qWarning() << "cannot init reencrypt process!" << dev << r;
        return -disk_encrypt::kErrorWrongPassphrase;   // might not pass wrong.
    }

    qInfo() << "processing decryption..." << dev;
    QPair<QString, QString> devInfo { dev, displayName };
    r = crypt_reencrypt_run(cdev,
                            crypt_setup_helper::onDecrypting,
                            (void *)&devInfo);
    qInfo() << "decryption process finished" << dev << r;
    if (r < 0) {
        qWarning() << "decrypt device failed!" << dev << r;
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
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
        return -disk_encrypt::kErrorLoadCrypt;
    }

    r = crypt_keyslot_add_by_passphrase(cdev,
                                        CRYPT_ANY_SLOT,
                                        validPwd.toStdString().c_str(),
                                        validPwd.length(),
                                        newPwd.toStdString().c_str(),
                                        newPwd.length());
    if (r < 0) {
        qWarning() << "change passphrase failed!" << dev << r;
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
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
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
        qWarning() << "change passphrase failed!" << dev << r;
        return -disk_encrypt::kErrorChangePassphraseFailed;
    }
    return r;   // the key slot number.
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
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
        return -disk_encrypt::kErrorLoadCrypt;
    }
    r = crypt_token_json_set(cdev,
                             tokenIndex,
                             token.toStdString().c_str());
    if (r < 0) {
        qWarning() << "cannot set token at index" << dev << r << tokenIndex;
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
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
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

    qInfo() << "token not found." << dev;
    return disk_encrypt::kSuccess;
}

int crypt_setup::csActivateDevice(const QString &dev, const QString &activateName, const QString &passphrase)
{
    struct crypt_device *cdev { nullptr };
    dfmbase::FinallyUtil atFinish([&] {if (cdev) crypt_free(cdev); });

    int r = crypt_init(&cdev,
                       dev.toStdString().c_str());
    if (r < 0) {
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
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
            qWarning() << "cannot activate device!" << dev << r;
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
        qWarning() << "cannot init crypt device" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
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
        qWarning() << "cannot get persistent flag!" << dev << r;
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
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev,
                   CRYPT_LUKS,
                   nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
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
            qWarning() << "cannot activate device!" << dev << r;
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
        qWarning() << "cannot init crypt device!" << dev << r;
        return -disk_encrypt::kErrorInitCrypt;
    }

    r = crypt_load(cdev, CRYPT_LUKS, nullptr);
    if (r < 0) {
        qWarning() << "cannot load crypt device!" << dev << r;
        return -disk_encrypt::kErrorLoadCrypt;
    }

    r = crypt_set_label(cdev, label.toStdString().c_str(), nullptr);
    if (r < 0) {
        qWarning() << "cannot set lable on device!" << label << dev << r;
        return -disk_encrypt::kErrorSetLabel;
    }
    return disk_encrypt::kSuccess;
}

int crypt_setup_helper::genDetachHeaderPath(const QString &dev, QString *name)
{
    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qWarning() << "cannot create device object!" << dev;
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
