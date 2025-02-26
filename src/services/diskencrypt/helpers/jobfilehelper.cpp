// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jobfilehelper.h"
#include "core/cryptsetup.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>

inline constexpr char kUSecFstabConfig[] { "/etc/usec-crypt/encrypt.json" };
inline constexpr char kUSecOtherConfig[] { "/etc/usec-crypt/encrypt_%1.json" };

FILE_ENCRYPT_USE_NS

int job_file_helper::createEncryptJobFile(const JobDescArgs &args)
{
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
    if (f.exists())
        qInfo() << "the old config will be replaced!" << fileName;

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "cannto create encrypt file!" << fileName;
        return -disk_encrypt::kErrorOpenFileFailed;
    }

    f.write(doc.toJson());
    f.flush();
    f.close();
    return 0;
}

int job_file_helper::createDecryptJobFile(const JobDescArgs &args)
{
    createUSecRoot();

    using namespace key_name;
    QJsonObject obj;
    obj.insert(KeyDevice, args.device);
    obj.insert(KeyDevPath, args.devPath);
    obj.insert(KeyDevType, args.devType);

    QJsonDocument doc(obj);
    QString fileName = QString(disk_encrypt::kUSecConfigDir) + "/decrypt.json";

    QFile f(fileName);
    if (f.exists())
        qInfo() << "the old config will be replaced!" << fileName;

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "cannto create encrypt file!" << fileName;
        return -disk_encrypt::kErrorOpenFileFailed;
    }

    f.write(doc.toJson());
    f.flush();
    f.close();

    return 0;
}

int job_file_helper::loadDecryptJobFile(JobDescArgs *args)
{
    return 0;
}

int job_file_helper::removeJobFile(const QString &jobFile)
{
    return ::remove(jobFile.toStdString().c_str());
}

int job_file_helper::loadEncryptJobFile(JobDescArgs *args, const QString &dev)
{
    if (!args)
        return 0;

    QString fileName = dev.isEmpty()
            ? kUSecFstabConfig
            : QString(kUSecOtherConfig).arg(dev.mid(5));

    QFile f(fileName);
    if (!f.exists()) {
        fileName.clear();
        qWarning() << "no fstab encrypt config exists!";

        QDir d(disk_encrypt::kUSecConfigDir);
        auto configs = d.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (configs.isEmpty())
            return 0;
        for (auto file : configs) {
            if (file.startsWith("encrypt") && file.endsWith(".json")) {
                fileName = QString(disk_encrypt::kUSecConfigDir) + "/" + file;
                break;
            }
        }

        if (fileName.isEmpty())
            return 0;
        qInfo() << "found unfinished job file:" << fileName;
    }

    f.setFileName(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "cannot open job file!" << fileName;
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
    return 0;
}

int job_file_helper::createUSecRoot()
{
    QDir d(disk_encrypt::kUSecConfigDir);
    if (d.exists())
        return 0;
    auto ok = d.mkpath(disk_encrypt::kUSecConfigDir);
    if (!ok) {
        qWarning() << "cannot create usec root!" << disk_encrypt::kUSecConfigDir;
        return -1;
    }
    return 0;
}

bool job_file_helper::hasJobFile()
{
    createUSecRoot();

    auto files = QDir(disk_encrypt::kUSecConfigDir).entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (auto file : files) {
        if (file.contains(QRegularExpression(R"((decrypt|encrypt|encrypt_.{3,})\.json)")))
            return true;
    }
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
    JobDescArgs job;
    loadEncryptJobFile(&job);
    if (job.jobFile.isEmpty())
        return;

    if (!QFile(job.devPath).exists()) {
        qInfo() << "job device does not exist!" << job.devPath
                << "about to remove job file" << job.jobFile;
        removeJobFile(job.jobFile);
        return;
    }

    if (crypt_setup_helper::encryptStatus(job.devPath) == disk_encrypt::kStatusNotEncrypted) {
        qInfo() << job.devPath << "is not encrypted"
                << "about to remove job file" << job.jobFile;
        removeJobFile(job.jobFile);
        return;
    }
}
