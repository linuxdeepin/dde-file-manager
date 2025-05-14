
// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    auto items = cryptItems();
    if (items.isEmpty()) {
        qInfo() << "no item should be modified.";
        return true;
    }

    bool needUpdate = false;
    for (auto &item : items) {
        if (item.target == activeName
            && !item.options.contains(opt)) {
            item.options.append(opt);
            needUpdate = true;
            break;
        }
    }

    if (!needUpdate) {
        qInfo() << "target device not found or item no need to be updated." << activeName << opt;
        return false;
    }
    qInfo() << "about to update crypttab." << activeName << opt;
    saveCryptItems(items);
    return true;
}

bool crypttab_helper::updateCryptTab()
{
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
            qWarning() << "cannot create device by" << item.source
                       << "keep it in crypttab";
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
        qInfo() << item.source << "is not encrypted anymore, remove from crypttab.";
        needUpdate = true;
    }

    if (needUpdate)
        saveCryptItems(newItems);
    return true;
}

bool crypttab_helper::insertCryptItem(const CryptItem &item)
{
    auto items = cryptItems();
    for (int i = items.count() - 1; i >= 0; --i) {
        if (items.at(i).source == item.source) {
            qInfo() << "duplicated item, will be removed..." << item.source << item.target;
            items.removeAt(i);
        }
    }
    items.append(item);

    qInfo() << "new crypt item is added." << item.source << item.target;
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
    bool merged = false;
    auto backupItems = cryptItems(kBackupCryptTabPath);
    auto currentItems = cryptItems();

    QSet<QString> currentItemTargets;
    for (const auto &item : currentItems) {
        currentItemTargets.insert(item.target);
    }

    for (const auto &backupItem : backupItems) {
        // 如果有完全相同的项或相同的 target，则跳过
        if (currentItems.contains(backupItem) || currentItemTargets.contains(backupItem.target))
            continue;

        // 否则，添加到 currentItems
        currentItems.append(backupItem);
        currentItemTargets.insert(backupItem.target);
        qInfo() << "new crypt item is added." << backupItem.source << backupItem.target;
        merged = true;
    }

    if (merged) {
        // updateInitramfs 会在 updateCryptTab 中调用
        saveCryptItems(currentItems, false);
    }

    // 更新加密设备配置文件
    updateCryptTab();
    return merged;
}

QList<crypttab_helper::CryptItem> crypttab_helper::cryptItems(const QString &crypttabFile)
{
    auto path = crypttabFile.isEmpty() ? kCryptTabPath : crypttabFile;
    QFile f(path);
    if (!f.exists()) {
        qWarning() << path << "not exist!";
        return {};
    }

    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "cannot open crypttab for read!";
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
            qInfo() << "found invalid crypt line" << item;
            continue;
        }

        ret.append({ fields.at(0),
                     fields.at(1),
                     fields.at(2),
                     fields.at(3).split(',', Qt::SkipEmptyParts) });
    }

    return ret;
}

void crypttab_helper::saveCryptItems(const QList<CryptItem> &items, bool doUpdateInitramfs)
{
    QFile f(kCryptTabPath);
    if (!f.exists()) {
        qWarning() << kCryptTabPath << "not exist!";
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
        qWarning() << "cannot open crypttab for write!";
        return;
    }

    f.write(data);
    f.flush();
    f.close();

    if (doUpdateInitramfs)
        updateInitramfs();
}

void crypttab_helper::updateInitramfs()
{
    // QtConcurrent::run([]{
    auto fd = inhibit_helper::inhibit("Updating initramfs...");
    qInfo() << "start update initramfs...";
    system("update-initramfs -u");
    qInfo() << "initramfs updated.";
    // });
}

bool crypttab_helper::removeCryptItem(const QString &activeName)
{
    bool removed = false;
    auto items = cryptItems();
    for (int i = items.count() - 1; i >= 0; --i) {
        if (items[i].target == activeName) {
            items.removeAt(i);
            removed = true;
            break;
        }
    }

    if (removed)
        saveCryptItems(items);
    return true;
}
