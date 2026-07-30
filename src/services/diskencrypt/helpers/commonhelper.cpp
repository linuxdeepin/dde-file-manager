// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonhelper.h"
#include "globaltypesdefine.h"

#include <dfm-base/utils/finallyutil.h>

#include <QFile>
#include <QLibrary>
#include <QRandomGenerator>

#include <DConfig>

FILE_ENCRYPT_USE_NS

void common_helper::createDFMDesktopEntry()
{
    // The desktop file is shipped via the package (installed to
    // /usr/share/applications/dfm-reencrypt.desktop). There is no need to
    // create it at runtime anymore -- writing to /usr at runtime fails on
    // immutable systems (磐石) and triggers security interception.
    // Keep this function as a no-op compatibility stub so existing callers
    // (DiskEncryptSetupPrivate::initialize) stay unaffected.
    if (!QFile::exists(disk_encrypt::kReencryptDesktopFile)) {
        qWarning() << "[common_helper::createDFMDesktopEntry] Reencrypt desktop file is missing,"
                   << "it should be shipped by the package:" << disk_encrypt::kReencryptDesktopFile;
    }
}

QString common_helper::encryptCipher()
{
    qInfo() << "[common_helper::encryptCipher] Getting encryption cipher configuration";

    auto cfg = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                          "org.deepin.dde.file-manager.diskencrypt");
    cfg->deleteLater();
    auto cipher = cfg->value("encryptAlgorithm", "aes").toString();

    QStringList supportedCipher { "sm4", "aes" };
    if (!supportedCipher.contains(cipher)) {
        qWarning() << "[common_helper::encryptCipher] Unsupported cipher algorithm, using default:" << cipher << "-> aes";
        return "aes";
    }

    qInfo() << "[common_helper::encryptCipher] Using encryption cipher:" << cipher;
    return cipher;
}

void common_helper::createRebootFlagFile(const QString &dev)
{
    qInfo() << "[common_helper::createRebootFlagFile] Creating reboot flag file for device:" << dev;

    QString fileName = disk_encrypt::kRebootFlagFilePrefix + dev.mid(5);
    QFile f(fileName);
    if (!f.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        qCritical() << "[common_helper::createRebootFlagFile] Failed to create reboot flag file:" << fileName;
        return;
    }
    f.close();
    qInfo() << "[common_helper::createRebootFlagFile] Reboot flag file created successfully:" << fileName;
}

QString common_helper::genRecoveryKey()
{
    qInfo() << "[common_helper::genRecoveryKey] Generating recovery key";

    QString recKey;
    QLibrary lib("usec-recoverykey");
    dfmbase::FinallyUtil finalClear([&] { if (lib.isLoaded()) lib.unload(); });

    if (!lib.load()) {
        qWarning() << "[common_helper::genRecoveryKey] Failed to load libusec-recoverykey, using default generator";
        recKey = genRandomString();
        qInfo() << "[common_helper::genRecoveryKey] Recovery key generated using default method, length:" << recKey.length();
        return recKey;
    }

    typedef int (*FnGenKey)(char *, const size_t, const size_t);
    FnGenKey fn = (FnGenKey)(lib.resolve("usec_get_recovery_key"));
    if (!fn) {
        qWarning() << "[common_helper::genRecoveryKey] Failed to resolve libusec-recoverykey function, using random string";
        recKey = genRandomString();
        qInfo() << "[common_helper::genRecoveryKey] Recovery key generated using random method, length:" << recKey.length();
        return recKey;
    }

    static const size_t kRecoveryKeySize = 24;
    char genKey[kRecoveryKeySize + 1];
    int ret = fn(genKey, kRecoveryKeySize, 1);
    if (ret != 0) {
        qWarning() << "[common_helper::genRecoveryKey] Failed to generate recovery key via library, error code:" << ret << "using random string";
        recKey = genRandomString();
        qInfo() << "[common_helper::genRecoveryKey] Recovery key generated using fallback method, length:" << recKey.length();
        return recKey;
    }

    recKey = genKey;
    qInfo() << "[common_helper::genRecoveryKey] Recovery key generated successfully via library, length:" << recKey.length();
    return recKey;
}

QString common_helper::genRandomString(int len)
{
    qDebug() << "[common_helper::genRandomString] Generating random string with length:" << len;

    // 定义字符集
    const QString charset = QString("0123456789"
                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz");

    QString result;
    result.reserve(len);

    // 获取全局随机生成器实例
    QRandomGenerator *generator = QRandomGenerator::global();

    // 生成随机字符串
    for (int i = 0; i < len; ++i) {
        int index = generator->bounded(charset.length());
        result.append(charset.at(index));
    }

    qDebug() << "[common_helper::genRandomString] Random string generated successfully, length:" << result.length();
    return result;
}
