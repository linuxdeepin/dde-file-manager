// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "normaldecryptworker.h"
#include "core/cryptsetup.h"
#include "helpers/inhibithelper.h"

FILE_ENCRYPT_USE_NS

NormalDecryptWorker::NormalDecryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void NormalDecryptWorker::run()
{
    qInfo() << "==> NormalDecryptWorker::run()";

    using namespace disk_encrypt::encrypt_param_keys;
    auto devPath = m_args.value(kKeyDevice, "").toString();
    qInfo() << "About to decrypt normal device:" << devPath;

    auto fd = inhibit_helper::inhibit(tr("Decrypting ") + devPath);

    auto devPass = disk_encrypt::fromBase64(m_args.value(kKeyPassphrase, "").toString());
    auto devName = m_args.value(kKeyDeviceName, "").toString();
    qDebug() << "Device name:" << devName;

    auto status = crypt_setup_helper::encryptStatus(devPath);
    if (status & disk_encrypt::kStatusOnline && status & disk_encrypt::kStatusEncrypt) {
        qCritical() << "Device is not fully encrypted, cannot decrypt, device:" << devPath << "status:" << status;
        setExitCode(-disk_encrypt::kErrorNotFullyEncrypted);
        return;
    }

    int r = crypt_setup::csDecryptMoveHead(devPath, devPass, devName);
    if (r < 0) {
        qCritical() << "Device decrypt failed, device:" << devPath << "error:" << r;
        setExitCode(r);
        return;
    }

    qInfo() << "Normal device decrypted successfully, device:" << devPath;
}
