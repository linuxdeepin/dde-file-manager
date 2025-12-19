// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jobfilehelper.h"
#include "core/cryptsetup.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>

#include <unistd.h>
#include <cerrno>

inline constexpr char kUSecFstabConfig[] { "/etc/usec-crypt/encrypt.json" };
inline constexpr char kUSecOtherConfig[] { "/etc/usec-crypt/encrypt_%1.json" };

FILE_ENCRYPT_USE_NS

namespace {
// Ensure file data is synced to disk
inline void syncToDisk(QFile &f)
{
    f.flush();
    int fd = f.handle();
    if (fd < 0) {
        qWarning() << "[syncToDisk] Invalid file handle:" << fd;
        return;
    }
    int ret = ::fsync(fd);
    if (ret != 0)
        qWarning() << "[syncToDisk] fsync failed, errno:" << errno;
    else
        qDebug() << "[syncToDisk] fsync succeeded for fd:" << fd;
}
}  // namespace

int job_file_helper::createEncryptJobFile(JobDescArgs &args)
{
    qInfo() << "[job_file_helper::createEncryptJobFile] Creating encryption job file for device:" << args.device << "type:" << args.devType;
    
    createUSecRoot();

    using namespace key_name;
    QJsonObject obj;
    obj.insert(KeyDevice, args.device);
    obj.insert(KeyVolume, args.volume);
    obj.insert(KeyCipher, args.cipher);
    obj.insert(KeyKeySze, args.keySze);
    obj.insert(KeyDevPath, args.devPath);
    obj.insert(KeyPrefferPath, args.prefferPath);
    obj.insert(KeyDevType, args.devType);
    obj.insert(KeyDevName, args.devName);
    obj.insert(KeyClearDev, args.clearDev);

    QJsonDocument doc(obj);
    QString fileName = disk_encrypt::kUSecConfigDir;
    if (args.devType == disk_encrypt::job_type::TypeFstab)
        fileName += "/encrypt.json";
    else
        fileName += QString("/encrypt_%1.json").arg(args.devPath.mid(5));

    QFile f(fileName);
    if (f.exists()) {
        qInfo() << "[job_file_helper::createEncryptJobFile] Replacing existing job file:" << fileName;
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "[job_file_helper::createEncryptJobFile] Failed to create encryption job file:" << fileName;
        return -disk_encrypt::kErrorOpenFileFailed;
    }

    args.jobFile = fileName;

    f.write(doc.toJson());
    syncToDisk(f);
    f.close();

    qInfo() << "[job_file_helper::createEncryptJobFile] Encryption job file created successfully:" << fileName;
    return 0;
}

int job_file_helper::createDecryptJobFile(JobDescArgs &args)
{
    qInfo() << "[job_file_helper::createDecryptJobFile] Creating decryption job file for device:" << args.device << "type:" << args.devType;
    
    createUSecRoot();

    using namespace key_name;
    QJsonObject obj;
    obj.insert(KeyDevice, args.device);
    obj.insert(KeyDevPath, args.devPath);
    obj.insert(KeyDevType, args.devType);

    QJsonDocument doc(obj);
    QString fileName = QString(disk_encrypt::kUSecConfigDir) + "/decrypt.json";

    QFile f(fileName);
    if (f.exists()) {
        qInfo() << "[job_file_helper::createDecryptJobFile] Replacing existing decryption job file:" << fileName;
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "[job_file_helper::createDecryptJobFile] Failed to create decryption job file:" << fileName;
        return -disk_encrypt::kErrorOpenFileFailed;
    }

    f.write(doc.toJson());
    syncToDisk(f);
    f.close();

    qInfo() << "[job_file_helper::createDecryptJobFile] Decryption job file created successfully:" << fileName;
    return 0;
}

// int job_file_helper::loadDecryptJobFile(JobDescArgs *args)
// {
//     // TODO: this function is not needed for now.
//     return 0;
// }

int job_file_helper::removeJobFile(const QString &jobFile)
{
    return ::remove(jobFile.toStdString().c_str());
}

int job_file_helper::loadEncryptJobFile(JobDescArgs *args, const QString &dev)
{
    qInfo() << "[job_file_helper::loadEncryptJobFile] Loading encryption job file for device:" << dev;
    
    if (!args) {
        qWarning() << "[job_file_helper::loadEncryptJobFile] Null arguments provided";
        return 0;
    }

    QString fileName = dev.isEmpty()
            ? kUSecFstabConfig
            : QString(kUSecOtherConfig).arg(dev.mid(5));

    QFile f(fileName);
    if (!f.exists()) {
        fileName.clear();
        qWarning() << "[job_file_helper::loadEncryptJobFile] Specific job file does not exist, searching for alternatives";

        QDir d(disk_encrypt::kUSecConfigDir);
        auto configs = d.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (configs.isEmpty()) {
            qInfo() << "[job_file_helper::loadEncryptJobFile] No job files found in USec config directory";
            return 0;
        }
        
        for (auto file : configs) {
            if (file.startsWith("encrypt") && file.endsWith(".json")) {
                fileName = QString(disk_encrypt::kUSecConfigDir) + "/" + file;
                break;
            }
        }

        if (fileName.isEmpty()) {
            qInfo() << "[job_file_helper::loadEncryptJobFile] No valid encryption job files found";
            return 0;
        }
        qInfo() << "[job_file_helper::loadEncryptJobFile] Found unfinished job file:" << fileName;
    } else {
        qInfo() << "[job_file_helper::loadEncryptJobFile] Loading job file:" << fileName;
    }

    f.setFileName(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        qCritical() << "[job_file_helper::loadEncryptJobFile] Failed to open job file for reading:" << fileName;
        return -disk_encrypt::kErrorOpenFileFailed;
    }
    
    auto json = f.readAll();
    f.close();

    auto doc = QJsonDocument::fromJson(json);
    auto obj = doc.object();

    auto value = [&](const QString &key) {
        return obj.value(key).toString();
    };
    args->device = value(key_name::KeyDevice);
    args->volume = value(key_name::KeyVolume);
    args->cipher = value(key_name::KeyCipher);
    args->keySze = value(key_name::KeyKeySze);
    args->devPath = value(key_name::KeyDevPath);
    args->prefferPath = value(key_name::KeyPrefferPath);
    args->devType = value(key_name::KeyDevType);
    args->devName = value(key_name::KeyDevName);
    args->clearDev = value(key_name::KeyClearDev);
    args->jobFile = fileName;
    
    qInfo() << "[job_file_helper::loadEncryptJobFile] Job file loaded successfully - device:" << args->device << "type:" << args->devType;
    return 0;
}

int job_file_helper::createUSecRoot()
{
    QDir d(disk_encrypt::kUSecConfigDir);
    if (d.exists()) {
        return 0;
    }
    
    qInfo() << "[job_file_helper::createUSecRoot] Creating USec root directory:" << disk_encrypt::kUSecConfigDir;
    auto ok = d.mkpath(disk_encrypt::kUSecConfigDir);
    if (!ok) {
        qCritical() << "[job_file_helper::createUSecRoot] Failed to create USec root directory:" << disk_encrypt::kUSecConfigDir;
        return -1;
    }
    
    qInfo() << "[job_file_helper::createUSecRoot] USec root directory created successfully:" << disk_encrypt::kUSecConfigDir;
    return 0;
}

bool job_file_helper::hasJobFile()
{
    qDebug() << "[job_file_helper::hasJobFile] Checking for existing job files";
    
    createUSecRoot();

    auto files = QDir(disk_encrypt::kUSecConfigDir).entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (auto file : files) {
        if (file.contains(QRegularExpression(R"((decrypt|encrypt|encrypt_.{3,})\.json)"))) {
            qInfo() << "[job_file_helper::hasJobFile] Found job file:" << file;
            return true;
        }
    }
    
    qDebug() << "[job_file_helper::hasJobFile] No job files found";
    return false;
}

QStringList job_file_helper::validJobTypes()
{
    return {
        disk_encrypt::job_type::TypeFstab,
        disk_encrypt::job_type::TypeNormal,
        disk_encrypt::job_type::TypeOverlay
    };
}

void job_file_helper::checkJobs()
{
    qInfo() << "[job_file_helper::checkJobs] Checking and validating existing job files";
    
    JobDescArgs job;
    loadEncryptJobFile(&job);
    if (job.jobFile.isEmpty()) {
        qInfo() << "[job_file_helper::checkJobs] No job files to check";
        return;
    }

    if (!QFile(job.devPath).exists()) {
        qInfo() << "[job_file_helper::checkJobs] Job device no longer exists, removing job file - device:" << job.devPath << "job file:" << job.jobFile;
        removeJobFile(job.jobFile);
        return;
    }

    if (crypt_setup_helper::encryptStatus(job.devPath) == disk_encrypt::kStatusNotEncrypted) {
        qInfo() << "[job_file_helper::checkJobs] Device is no longer encrypted, removing job file - device:" << job.devPath << "job file:" << job.jobFile;
        removeJobFile(job.jobFile);
        return;
    }
    
    qInfo() << "[job_file_helper::checkJobs] Job file validation completed - device:" << job.devPath << "job file:" << job.jobFile;
}
