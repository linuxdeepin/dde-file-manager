// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonhelper.h"
#include "globaltypesdefine.h"

#include <dfm-base/utils/finallyutil.h>

#include <QFile>
#include <QLibrary>
#include <QRandomGenerator>
#include <QDir>

#include <DConfig>
#include <unistd.h>

FILE_ENCRYPT_USE_NS

void common_helper::createDFMDesktopEntry()
{
    qInfo() << "[common_helper::createDFMDesktopEntry] Creating DFM desktop entry for reencryption";

    const QString &kLocalShareApps = "/usr/local/share/applications";
    QDir d(kLocalShareApps);
    if (!d.exists()) {
        auto ok = d.mkpath(kLocalShareApps);
        qInfo() << "[common_helper::createDFMDesktopEntry] Applications directory created:" << kLocalShareApps << "success:" << ok;
    }

    QFile f(disk_encrypt::kReencryptDesktopFile);
    if (f.exists()) {
        qInfo() << "[common_helper::createDFMDesktopEntry] Desktop file already exists, skipping creation:" << disk_encrypt::kReencryptDesktopFile;
        return;
    }

    QByteArray desktop {
        "[Desktop Entry]\n"
        "Categories=System;\n"
        "Comment=To auto launch reencryption\n"
        "Exec=/usr/libexec/dde-file-manager -d\n"
        "GenericName=Disk Reencrypt\n"
        "Icon=dde-file-manager\n"
        "Name=Disk Reencrypt\n"
        "Terminal=false\n"
        "Type=Application\n"
        "NoDisplay=true\n"
        "X-AppStream-Ignore=true\n"
        "X-Deepin-AppID=dde-file-manager\n"
        "X-Deepin-Vendor=deepin\n"
    };

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "[common_helper::createDFMDesktopEntry] Failed to open desktop file for writing:" << disk_encrypt::kReencryptDesktopFile;
        return;
    }
    f.write(desktop);
    f.flush();
    auto ret = ::fsync(f.handle());
    f.close();

    qInfo() << "[common_helper::createDFMDesktopEntry] Desktop file created successfully:" << disk_encrypt::kReencryptDesktopFile
            << "Sync status: " << ret;
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
