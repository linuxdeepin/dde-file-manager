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
    auto fd = inhibit_helper::inhibit(tr("Decrypting..."));

    using namespace disk_encrypt::encrypt_param_keys;
    auto devPath = m_args.value(kKeyDevice, "").toString();
    auto devPass = disk_encrypt::fromBase64(m_args.value(kKeyPassphrase, "").toString());
    auto devName = m_args.value(kKeyDeviceName, "").toString();

    auto status = crypt_setup_helper::encryptStatus(devPath);
    if (status & disk_encrypt::kStatusOnline && status & disk_encrypt::kStatusEncrypt) {
        qWarning() << "device is not fully encrypted, cannot decrypt!" << devPath;
        setExitCode(-disk_encrypt::kErrorNotFullyEncrypted);
        return;
    }

    int r = crypt_setup::csDecrypt(devPath, devPass, devName);
    if (r < 0) {
        qWarning() << "device decrypt failed!" << devPath;
        setExitCode(r);
        return;
    }
}
