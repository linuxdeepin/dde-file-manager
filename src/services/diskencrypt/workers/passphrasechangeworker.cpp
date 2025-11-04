// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    using namespace disk_encrypt::encrypt_param_keys;
    auto devPath = m_args.value(kKeyDevice, "").toString();
    if (devPath.startsWith("/dev/dm-")) {
        auto ptr = blockdev_helper::createDevPtr(devPath);
        if (!ptr) {
            qWarning() << "create device object failed!" << ptr;
            setExitCode(-disk_encrypt::kErrorUnknown);
            return;
        }
        qInfo() << "mapper device is passed, using physical device instead." << devPath;
        devPath = dm_setup_helper::findHolderDev(devPath);
        qInfo() << "physical device path is" << devPath;
    }

    auto oldPass = disk_encrypt::fromBase64(m_args.value(kKeyOldPassphrase, "").toString());
    auto newPass = disk_encrypt::fromBase64(m_args.value(kKeyPassphrase, "").toString());
    auto tpmToken = m_args.value(kKeyTPMToken, "").toString();
    auto useRec = m_args.value(kKeyValidateWithRecKey, false).toBool();

    int r = 0;
    if (useRec) {
        // 使用恢复密钥修改密码的流程：
        // 1. 先获取恢复密钥所在槽位
        QList<int> recoverySlots;
        r = crypt_setup_helper::getRecoveryKeySlots(devPath, &recoverySlots);
        if (r < 0) {
            qWarning() << "failed to get recovery key slots!" << devPath;
            setExitCode(r);
            return;
        }

        // 2. 添加新密码，得到新槽位
        r = crypt_setup::csAddPassphrase(devPath, oldPass, newPass);
        if (r < 0) {
            qWarning() << "add passphrase failed!" << devPath;
            setExitCode(r);
            return;
        }
        int newSlot = r;
        qInfo() << "new passphrase added to slot:" << newSlot;

        // 3. 删除除了恢复密钥和新密钥以外的所有槽位
        // LUKS 有 8 个槽位（0-7）
        for (int slot = 0; slot < 8; ++slot) {
            // 跳过恢复密钥槽位和新密码槽位
            if (recoverySlots.contains(slot) || slot == newSlot) {
                qInfo() << "keeping slot:" << slot;
                continue;
            }

            // 删除其他槽位
            int removeResult = crypt_setup::csRemoveKeyslot(devPath, slot);
            if (removeResult < 0) {
                // 槽位可能本来就是空的，不算错误
                qDebug() << "slot" << slot << "removal failed or already empty, ignoring";
            }
        }

        qInfo() << "old passphrases cleaned up, only recovery key and new passphrase remain.";
    }
    else
        r = crypt_setup::csChangePassphrase(devPath, oldPass, newPass);

    if (r < 0) {
        qWarning() << "modify passphrase for device failed!" << devPath;
        setExitCode(r);
        return;
    }

    qInfo() << "passphrase for device is changed." << devPath;

    if (tpmToken.isEmpty())
        return;

    auto doc = QJsonDocument::fromJson(tpmToken.toLocal8Bit());
    auto obj = doc.object();
    obj.insert("keyslots", QJsonArray::fromStringList({ QString::number(r) }));
    doc.setObject(obj);
    auto tk = doc.toJson(QJsonDocument::Compact);
    r = crypt_setup_helper::setToken(devPath, tk);
    if (r < 0) {
        qWarning() << "token set failed!" << devPath << r;
        setExitCode(r);
        return;
    }
    qInfo() << "device tpm token is updated." << devPath;
}
