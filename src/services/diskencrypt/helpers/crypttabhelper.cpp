// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/cryptsetup.h"
#include "crypttabhelper.h"
#include "blockdevhelper.h"
#include "inhibithelper.h"

#include <QFile>
#include <QRegularExpression>
#include <QtConcurrent>

FILE_ENCRYPT_USE_NS

static constexpr char kCryptTabPath[] { "/etc/crypttab" };
static constexpr char kBackupCryptTabPath[] { "/boot/usec-crypt/config/crypttab" };

bool crypttab_helper::addCryptOption(const QString &activeName, const QString &opt)
{
    qInfo() << "[crypttab_helper::addCryptOption] Adding crypt option:" << opt << "to device:" << activeName;
    
    auto items = cryptItems();
    if (items.isEmpty()) {
        qInfo() << "[crypttab_helper::addCryptOption] No crypt items found, nothing to modify";
        return true;
    }

    bool needUpdate = false;
    for (auto &item : items) {
        if (item.target == activeName
            && !item.options.contains(opt)) {
            item.options.append(opt);
            needUpdate = true;
            qInfo() << "[crypttab_helper::addCryptOption] Option added to device:" << activeName << "option:" << opt;
            break;
        }
    }

    if (!needUpdate) {
        qInfo() << "[crypttab_helper::addCryptOption] Target device not found or option already exists:" << activeName << "option:" << opt;
        return false;
    }
    
    qInfo() << "[crypttab_helper::addCryptOption] Updating crypttab with new option:" << activeName << "option:" << opt;
    saveCryptItems(items);
    return true;
}

bool crypttab_helper::updateCryptTab()
{
    qInfo() << "[crypttab_helper::updateCryptTab] Updating crypttab to remove non-encrypted devices";
    
    auto items = cryptItems();
    QList<CryptItem> newItems;
    bool needUpdate { false };
    
    for (auto item : items) {
        if (item.target.startsWith("#")) {
            newItems.append(item);
            continue;
        }
        
        auto dev = blockdev_helper::resolveDevObjPath(item.source);
        auto devptr = blockdev_helper::createDevPtr2(dev);
        if (!devptr) {
            qWarning() << "[crypttab_helper::updateCryptTab] Failed to create device for source:" << item.source << "keeping in crypttab";
            newItems.append(item);
            continue;
        }

        QString detachHeaderName;
        crypt_setup_helper::genDetachHeaderPath(devptr->device(), &detachHeaderName);

        // has header or detached header, treat as encrypted device.
        if (devptr->isEncrypted() || (!detachHeaderName.isEmpty() && QFile::exists(detachHeaderName))) {
            newItems.append(item);
            continue;
        }
        
        qInfo() << "[crypttab_helper::updateCryptTab] Device is no longer encrypted, removing from crypttab:" << item.source;
        needUpdate = true;
    }

    if (needUpdate) {
        qInfo() << "[crypttab_helper::updateCryptTab] Saving updated crypttab with" << newItems.size() << "items";
        saveCryptItems(newItems);
    } else {
        qInfo() << "[crypttab_helper::updateCryptTab] No changes needed for crypttab";
    }
    
    return true;
}

bool crypttab_helper::insertCryptItem(const CryptItem &item)
{
    qInfo() << "[crypttab_helper::insertCryptItem] Inserting crypt item - source:" << item.source << "target:" << item.target;
    
    auto items = cryptItems();
    for (int i = items.count() - 1; i >= 0; --i) {
        if (items.at(i).source == item.source) {
            qInfo() << "[crypttab_helper::insertCryptItem] Removing duplicate item:" << item.source << "target:" << item.target;
            items.removeAt(i);
        }
    }
    items.append(item);

    qInfo() << "[crypttab_helper::insertCryptItem] New crypt item added successfully:" << item.source << "target:" << item.target;
    saveCryptItems(items);
    return true;
}

/*
 * 合并加密设备配置文件
 * 1. 如果当前配置文件中存在完全相同的项，则跳过
 * 2. 如果当前配置文件中存在相同的 target，则跳过
 * 3. 否则，将备份配置文件中的项添加到当前配置文件中
 */
bool crypttab_helper::mergeCryptTab()
{
    qInfo() << "[crypttab_helper::mergeCryptTab] Merging crypttab configurations";
    
    bool merged = false;
    auto backupItems = cryptItems(kBackupCryptTabPath);
    auto currentItems = cryptItems();

    qInfo() << "[crypttab_helper::mergeCryptTab] Found" << backupItems.size() << "backup items and" << currentItems.size() << "current items";

    QSet<QString> currentItemTargets;
    for (const auto &item : currentItems) {
        currentItemTargets.insert(item.target);
    }

    for (const auto &backupItem : backupItems) {
        // 如果有完全相同的项或相同的 target，则跳过
        if (currentItems.contains(backupItem) || currentItemTargets.contains(backupItem.target)) {
            qDebug() << "[crypttab_helper::mergeCryptTab] Skipping duplicate or existing target:" << backupItem.target;
            continue;
        }

        // 否则，添加到 currentItems
        currentItems.append(backupItem);
        currentItemTargets.insert(backupItem.target);
        qInfo() << "[crypttab_helper::mergeCryptTab] New crypt item merged from backup:" << backupItem.source << "target:" << backupItem.target;
        merged = true;
    }

    if (merged) {
        qInfo() << "[crypttab_helper::mergeCryptTab] Saving merged crypttab configuration";
        // updateInitramfs 会在 updateCryptTab 中调用
        saveCryptItems(currentItems, false);
    } else {
        qInfo() << "[crypttab_helper::mergeCryptTab] No new items to merge";
    }

    // 更新加密设备配置文件
    updateCryptTab();
    return merged;
}

QList<crypttab_helper::CryptItem> crypttab_helper::cryptItems(const QString &crypttabFile)
{
    auto path = crypttabFile.isEmpty() ? kCryptTabPath : crypttabFile;
    qInfo() << "[crypttab_helper::cryptItems] Reading crypttab file:" << path;
    
    QFile f(path);
    if (!f.exists()) {
        qWarning() << "[crypttab_helper::cryptItems] Crypttab file does not exist:" << path;
        return {};
    }

    if (!f.open(QIODevice::ReadOnly)) {
        qCritical() << "[crypttab_helper::cryptItems] Failed to open crypttab file for reading:" << path;
        return {};
    }
    
    auto items = f.readAll().split('\n');
    f.close();

    QList<CryptItem> ret;

    for (auto item : items) {
        if (item.isEmpty())
            continue;

        auto fields = QString(item).split(QRegularExpression(R"( |\t)"), Qt::SkipEmptyParts);
        if (fields.count() < 4) {
            qWarning() << "[crypttab_helper::cryptItems] Found invalid crypt line, skipping:" << item;
            continue;
        }

        ret.append({ fields.at(0),
                     fields.at(1),
                     fields.at(2),
                     fields.at(3).split(',', Qt::SkipEmptyParts) });
    }

    qInfo() << "[crypttab_helper::cryptItems] Successfully parsed" << ret.size() << "crypt items from:" << path;
    return ret;
}

void crypttab_helper::saveCryptItems(const QList<CryptItem> &items, bool doUpdateInitramfs)
{
    qInfo() << "[crypttab_helper::saveCryptItems] Saving" << items.size() << "crypt items to crypttab, update initramfs:" << doUpdateInitramfs;
    
    QFile f(kCryptTabPath);
    if (!f.exists()) {
        qCritical() << "[crypttab_helper::saveCryptItems] Crypttab file does not exist:" << kCryptTabPath;
        return;
    }

    QStringList contents;
    for (auto item : items) {
        auto line = item.target + '\t'
                + item.source + '\t'
                + item.keyFile + '\t'
                + item.options.join(',');
        contents.append(line);
    }
    auto data = contents.join('\n').toLocal8Bit();
    data.append('\n');

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "[crypttab_helper::saveCryptItems] Failed to open crypttab file for writing:" << kCryptTabPath;
        return;
    }

    f.write(data);
    f.flush();
    f.close();

    qInfo() << "[crypttab_helper::saveCryptItems] Crypttab file saved successfully with" << items.size() << "items";

    if (doUpdateInitramfs) {
        updateInitramfs();
    }
}

void crypttab_helper::updateInitramfs()
{
    qInfo() << "[crypttab_helper::updateInitramfs] Starting initramfs update process";
    
    // QtConcurrent::run([]{
    auto fd = inhibit_helper::inhibit("Updating initramfs...");
    qInfo() << "[crypttab_helper::updateInitramfs] System shutdown/sleep inhibited for initramfs update";
    
    int ret = std::system("sudo -E /sbin/update-initramfs -u");
    if (ret != 0) {
        qCritical() << "[crypttab_helper::updateInitramfs] Failed to update initramfs, error code:" << ret;
    } else {
        qInfo() << "[crypttab_helper::updateInitramfs] Initramfs updated successfully";
    }
    // });
}

bool crypttab_helper::removeCryptItem(const QString &activeName)
{
    qInfo() << "[crypttab_helper::removeCryptItem] Removing crypt item with target:" << activeName;
    
    bool removed = false;
    auto items = cryptItems();
    for (int i = items.count() - 1; i >= 0; --i) {
        if (items[i].target == activeName) {
            items.removeAt(i);
            removed = true;
            qInfo() << "[crypttab_helper::removeCryptItem] Crypt item removed successfully:" << activeName;
            break;
        }
    }

    if (removed) {
        saveCryptItems(items);
    } else {
        qWarning() << "[crypttab_helper::removeCryptItem] Crypt item not found for removal:" << activeName;
    }
    
    return true;
}
