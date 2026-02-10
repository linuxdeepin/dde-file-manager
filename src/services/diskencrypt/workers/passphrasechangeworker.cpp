// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "passphrasechangeworker.h"
#include "core/cryptsetup.h"
#include "core/dmsetup.h"
#include "helpers/blockdevhelper.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

FILE_ENCRYPT_USE_NS

PassphraseChangeWorker::PassphraseChangeWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void PassphraseChangeWorker::run()
{
    qInfo() << "[PassphraseChangeWorker::run] Starting passphrase change operation";

    using namespace disk_encrypt::encrypt_param_keys;
    auto devPath = m_args.value(kKeyDevice, "").toString();
    qInfo() << "[PassphraseChangeWorker::run] Target device:" << devPath;

    if (devPath.startsWith("/dev/dm-")) {
        auto ptr = blockdev_helper::createDevPtr(devPath);
        if (!ptr) {
            qCritical() << "[PassphraseChangeWorker::run] Create device object failed, device:" << devPath;
            setExitCode(-disk_encrypt::kErrorUnknown);
            return;
        }
        qInfo() << "[PassphraseChangeWorker::run] Mapper device detected, resolving physical device:" << devPath;
        devPath = dm_setup_helper::findHolderDev(devPath);
        qInfo() << "[PassphraseChangeWorker::run] Physical device resolved:" << devPath;
    }

    auto oldPass = disk_encrypt::fromBase64(m_args.value(kKeyOldPassphrase, "").toString());
    auto newPass = disk_encrypt::fromBase64(m_args.value(kKeyPassphrase, "").toString());
    auto tpmToken = m_args.value(kKeyTPMToken, "").toString();
    auto useRec = m_args.value(kKeyValidateWithRecKey, false).toBool();
    qDebug() << "[PassphraseChangeWorker::run] Use recovery key:" << useRec << ", has TPM token:" << !tpmToken.isEmpty();

    int r = 0;
    if (useRec) {
        qInfo() << "[PassphraseChangeWorker::run] Using recovery key to change passphrase";
        // 使用恢复密钥修改密码的流程：
        // 1. 先获取恢复密钥所在槽位
        QList<int> recoverySlots;
        r = crypt_setup_helper::getRecoveryKeySlots(devPath, &recoverySlots);
        if (r < 0) {
            qCritical() << "[PassphraseChangeWorker::run] Failed to get recovery key slots, device:" << devPath << "error:" << r;
            setExitCode(r);
            return;
        }
        qDebug() << "[PassphraseChangeWorker::run] Recovery key slots:" << recoverySlots;

        // 2. 添加新密码，得到新槽位
        r = crypt_setup::csAddPassphrase(devPath, oldPass, newPass);
        if (r < 0) {
            qCritical() << "[PassphraseChangeWorker::run] Add passphrase failed, device:" << devPath << "error:" << r;
            setExitCode(r);
            return;
        }
        int newSlot = r;
        qInfo() << "[PassphraseChangeWorker::run] New passphrase added to slot:" << newSlot;

        // 3. 删除除了恢复密钥和新密钥以外的所有槽位
        // LUKS 有 8 个槽位（0-7）
        qInfo() << "[PassphraseChangeWorker::run] Cleaning up old passphrase slots";
        for (int slot = 0; slot < 8; ++slot) {
            // 跳过恢复密钥槽位和新密码槽位
            if (recoverySlots.contains(slot) || slot == newSlot) {
                qDebug() << "[PassphraseChangeWorker::run] Keeping slot:" << slot;
                continue;
            }

            // 删除其他槽位
            int removeResult = crypt_setup::csRemoveKeyslot(devPath, slot);
            if (removeResult < 0) {
                // 槽位可能本来就是空的，不算错误
                qDebug() << "[PassphraseChangeWorker::run] Slot" << slot << "removal failed or already empty, ignoring";
            } else {
                qDebug() << "[PassphraseChangeWorker::run] Slot" << slot << "removed successfully";
            }
        }

        qInfo() << "[PassphraseChangeWorker::run] Old passphrases cleaned up, only recovery key and new passphrase remain";
    } else {
        qInfo() << "[PassphraseChangeWorker::run] Changing passphrase with old passphrase";
        r = crypt_setup::csChangePassphrase(devPath, oldPass, newPass);
    }

    if (r < 0) {
        qCritical() << "[PassphraseChangeWorker::run] Modify passphrase failed, device:" << devPath << "error:" << r;
        setExitCode(r);
        return;
    }

    qInfo() << "[PassphraseChangeWorker::run] Passphrase changed successfully, device:" << devPath;

    if (tpmToken.isEmpty()) {
        qDebug() << "[PassphraseChangeWorker::run] No TPM token to update";
        return;
    }

    qInfo() << "[PassphraseChangeWorker::run] Updating TPM token";
    auto doc = QJsonDocument::fromJson(tpmToken.toLocal8Bit());
    auto obj = doc.object();
    obj.insert("keyslots", QJsonArray::fromStringList({ QString::number(r) }));
    doc.setObject(obj);
    auto tk = doc.toJson(QJsonDocument::Compact);
    r = crypt_setup_helper::setToken(devPath, tk);
    if (r < 0) {
        qCritical() << "[PassphraseChangeWorker::run] TPM token set failed, device:" << devPath << "error:" << r;
        setExitCode(r);
        return;
    }
    qInfo() << "[PassphraseChangeWorker::run] TPM token updated successfully, device:" << devPath;
}
