// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    QFile f(disk_encrypt::kReencryptDesktopFile);
    if (f.exists())
        return;

    QByteArray desktop {
        "[Desktop Entry]\n"
        "Categories=System;\n"
        "Comment=To auto launch reencryption\n"
        "Exec=/usr/bin/dde-file-manager -d\n"
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
        qWarning() << "cannot open desktop file to write!";
        return;
    }
    f.write(desktop);
    f.close();

    qInfo() << "desktop file created.";
}

QString common_helper::encryptCipher()
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

void common_helper::createRebootFlagFile(const QString &dev)
{
    QString fileName = disk_encrypt::kRebootFlagFilePrefix + dev.mid(5);
    QFile f(fileName);
    if (!f.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        qWarning() << "cannot create reboot flag file";
        return;
    }
    f.close();
    qInfo() << "reboot flag created." << fileName;
}

QString common_helper::genRecoveryKey()
{
    QString recKey;
    QLibrary lib("usec-recoverykey");
    dfmbase::FinallyUtil finalClear([&] { if (lib.isLoaded()) lib.unload(); });

    if (!lib.load()) {
        qWarning() << "libusec-recoverykey load failed. use default generator";
        recKey = genRandomString();
        return recKey;
    }

    typedef int (*FnGenKey)(char *, const size_t, const size_t);
    FnGenKey fn = (FnGenKey)(lib.resolve("usec_get_recovery_key"));
    if (!fn) {
        qWarning() << "libusec-recoverykey resolve failed. use uuid as recovery key";
        return recKey;
    }

    static const size_t kRecoveryKeySize = 24;
    char genKey[kRecoveryKeySize + 1];
    int ret = fn(genKey, kRecoveryKeySize, 1);
    if (ret != 0) {
        qWarning() << "libusec-recoverykey generate failed. use uuid as recovery key";
        return recKey;
    }

    recKey = genKey;
    return recKey;
}

QString common_helper::genRandomString(int len)
{
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

    return result;
}
